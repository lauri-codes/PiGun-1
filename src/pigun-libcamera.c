#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <sys/mman.h>

#include <libcamera/libcamera.h>
#include <libcamera/framebuffer_allocator.h>

#include "pigun.h"
#include "pigun-detector.h"
#include "pigun-gpio.h"
#include "pigun-libcamera.h"

using namespace libcamera;
using namespace std::chrono_literals;

static std::shared_ptr<Camera> camera;

// Global file descriptor
static int g_hid_fd = -1;

// Suppose pigun is a global struct holding the data you want to send
// struct { ... } pigun;

bool init_hid() {
    g_hid_fd = open("/dev/hidg0", O_WRONLY);
    if (g_hid_fd < 0) {
        std::cerr << "Failed to open /dev/hidg0" << std::endl;
        return false;
    }
    return true;
}

void close_hid() {
    if (g_hid_fd >= 0) {
        close(g_hid_fd);
        g_hid_fd = -1;
    }
}


void printFPS() {
    using Clock = std::chrono::steady_clock;
    static auto lastTime = Clock::now();
    static int frames = 0;

    frames++;

    auto currentTime = Clock::now();
    auto elapsedTime = std::chrono::duration<double>(currentTime - lastTime).count();

    if (elapsedTime >= 1.0) {
        std::printf("FPS: %d\n", frames);
        frames = 0;
        lastTime = currentTime;
    }
}

int send_hid_interrupt_message() {
    // Report data structure
    uint8_t hid_report[] = {
        0xa1,            // a1 = device-to-host (from your original code)
        PIGUN_REPORT_ID, // or your joystick report ID
        0, 0, 0, 0,      // placeholders for X, Y
        0                // placeholder for buttons
    };

    // Fill with the actual data
	hid_report[2] = (pigun.report.x) & 0xff;
	hid_report[3] = (pigun.report.x >> 8) & 0xff;
	hid_report[4] = (pigun.report.y) & 0xff;
	hid_report[5] = (pigun.report.y >> 8) & 0xff;
	hid_report[6] = pigun.report.buttons;

    // Write the report
    if (write(g_hid_fd, hid_report, sizeof(hid_report)) < 0) {
        std::cerr << "Failed to write report to /dev/hidg0" << std::endl;
        return 1;
    }
    return 0;
}

static void requestComplete(Request *request)
{
    std::cout << "PROCESS FRAME" << std::endl;
    // If request was cancelled, ignore it as the data might be invalid
    if (request->status() == Request::RequestCancelled) {
        std::cout << "CANCELLED" << std::endl;
        return;
    }

    // If shutting down, stop frame requests
	if (pigun.state == STATE_SHUTDOWN){
        std::cout << "SHUTDOWN" << std::endl;
		return;
	}
     
    // Process completed buffers
    for (auto [stream, buffer] : request->buffers()) {
        std::cout << "PLANE" << std::endl;

        // Print FPS
        printFPS();

        // Extract frame data from the request. We only retrieve the Y
        // (luminance) plane
        // const auto &planes = buffer->planes();
        // const auto &yPlane = planes[0];
        // int     yFd      = yPlane.fd.get();
        // size_t  ySize    = yPlane.length;
        // size_t  yOffset  = yPlane.offset;
        // void   *yData    = mmap(nullptr, ySize, PROT_READ, MAP_SHARED, yFd, yOffset);
        // pigun.framedata = (uint8_t *)(yData);

        // // Call the peak detector function. If there was a detector error, error LED
        // // goes on, otherwise off.
        // uint8_t ce = pigun.detector.error;
        // pigun_detector_run(pigun.framedata);
        // if (pigun.detector.error != ce) {
        //     pigun_GPIO_output_set(PIN_OUT_ERR, pigun.detector.error);
        // }

        // // TODO: maybe add a mutex/semaphore so that the main bluetooth thread
        // // will wait until this is done with the x/y aim before reading the HID report

        // // compute aiming position from the detected peaks
        // pigun_calculate_aim();

        // // *********************************************************************
        // // check the buttons ***************************************************
        // pigun_buttons_process();

        // // Send HID report
        // send_hid_interrupt_message();
    }

    // Request new frame by reusing the buffer
    request->reuse(Request::ReuseBuffers);
    camera->queueRequest(request);
}

int pigun_libcamera_init()
{
    // Initialize the HID device once
    if (!init_hid()) {
        return 1;
    }

    // Create and start the CameraManager
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    // Acquire the camera
    auto cameras = cm->cameras();
    if (cameras.empty()) {
        std::cout << "No cameras were identified on the system." << std::endl;
        cm->stop();
        return EXIT_FAILURE;
    }
    std::string cameraId = cameras[0]->id();
    camera = cm->get(cameraId);
    camera->acquire();

    // Configure camera resolution and format
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::VideoRecording } );
    StreamConfiguration &streamConfig = config->at(0);
    streamConfig.size.width = PIGUN_RES_X; 
    streamConfig.size.height = PIGUN_RES_Y;
    streamConfig.pixelFormat = formats::YUV420;
    CameraConfiguration::Status status = config->validate();
    if (status == CameraConfiguration::Status::Invalid) {
        std::cerr << "Configuration invalid" << std::endl;
        return -1;
    }
    if (camera->configure(config.get()) < 0) {
        std::cerr << "Failed to configure camera" << std::endl;
        return -1;
    }

    // Set FPS
    std::unique_ptr<libcamera::ControlList> camcontrols = std::unique_ptr<libcamera::ControlList>(new libcamera::ControlList());
    int64_t frameDuration = 1000000ULL / PIGUN_FPS; // 8333 µs
    camcontrols->set(controls::FrameDurationLimits, libcamera::Span<const std::int64_t, 2>({frameDuration, frameDuration}));

    // Use a FrameBufferAllocator to allocate buffers (not fully shown)
    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);
    Stream *stream = streamConfig.stream();
    if (allocator->allocate(stream) < 0) {
        std::cerr << "Failed to allocate buffers" << std::endl;
        return -1;
    }

    // Create a list of original requests to process
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    std::vector<std::unique_ptr<Request>> requests;
    for (unsigned int i = 0; i < buffers.size(); ++i) {
        std::unique_ptr<Request> request = camera->createRequest();
        if (!request)
        {
            std::cerr << "Can't create request" << std::endl;
            return -ENOMEM;
        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        int ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Can't set buffer for request" << std::endl;
            return ret;
        }

        requests.push_back(std::move(request));
    }

    // Set callback for frame request completion
    camera->requestCompleted.connect(requestComplete);

    // Start camera and queue requests
    camera->start(camcontrols.get());
    for (std::unique_ptr<Request> &request : requests) {
        camera->queueRequest(request.get());
    }

    // Sleep for 5 minutes
    std::this_thread::sleep_for(30000ms);

    // Stop camera and release resources
    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();
    close_hid();
    return 0;
}