#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include <libcamera/libcamera.h>
#include <libcamera/framebuffer_allocator.h>

#include "pigun.h"
#include "pigun-detector.h"
#include "pigun-gpio.h"
#include "pigun-libcamera.h"

using namespace libcamera;
using namespace std::chrono_literals;

static std::shared_ptr<Camera> camera;

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

static void requestComplete(Request *request)
{
    // If request was cancelled, ignore it as the data might be invalid
    if (request->status() == Request::RequestCancelled) {
        return;
    }

    // If shutting down, stop frame requests
	if (pigun.state == STATE_SHUTDOWN){
		return;
	}
     
    // Process completed buffers
    for (auto [stream, buffer] : request->buffers()) {
        // Print FPS
        printFPS();

        // Extract frame data from the request
        MappedFrameBuffer mappedBuffer(buffer, MappedFrameBuffer::MapFlag::Read);
        const std::vector<Span<uint8_t>> mem = mappedBuffer.planes();
        pirung.framedata = (uint8_t *)(mem[0].data())
        // pigun.framedata = buffer->data;

        // Call the peak detector function. If there was a detector error, error LED
        // goes on, otherwise off.
        uint8_t ce = pigun.detector.error;
        pigun_detector_run(pigun.framedata);
        if (pigun.detector.error != ce) {
            pigun_GPIO_output_set(PIN_OUT_ERR, pigun.detector.error);
        }

        // TODO: maybe add a mutex/semaphore so that the main bluetooth thread
        // will wait until this is done with the x/y aim before reading the HID report

        // compute aiming position from the detected peaks
        pigun_calculate_aim();

        // *********************************************************************
        // check the buttons ***************************************************
        pigun_buttons_process();

        // TODO: maybe add a mutex/semaphore so that the main bluetooth thread
        // will wait until this is done with the buttons before reading the HID report
        // *********************************************************************
    }

    // Request new frame by reusing the buffer
    request->reuse(Request::ReuseBuffers);
    camera->queueRequest(request);
}

int pigun_libcamera_init()
{
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

    // Wait for 30 seconds
    std::this_thread::sleep_for(30000ms);

    // Stop camera and release resources
    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();
    return 0;
}