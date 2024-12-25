#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <time.h>

#include <libcamera/libcamera.h>
#include <libcamera/framebuffer_allocator.h>

using namespace libcamera;
using namespace std::chrono_literals;

static std::shared_ptr<Camera> camera;

void printFPS() {
    static int frames = 0;
    static double lastTime = 0.0;

    // Get the current time
    double currentTime = (double)clock() / CLOCKS_PER_SEC;
    frames++;

    // Calculate FPS every second
    if (currentTime - lastTime >= 1.0) {
        printf("FPS: %d\n", frames);
        
        // Reset the frame count and lastTime
        frames = 0;
        lastTime = currentTime;
    }
}

static void requestComplete(Request *request)
{
    printFPS();
    request->reuse(Request::ReuseBuffers);
    camera->queueRequest(request);
}

int main()
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
    auto camera = cm->get(cameraId);
    camera->acquire();

    // Configure camera
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::VideoRecording } );
    StreamConfiguration &streamConfig = config->at(0);
    streamConfig.size.width = 640; 
    streamConfig.size.height = 480;
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

    // Use a FrameBufferAllocator to allocate buffers (not fully shown)
    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);
    Stream *stream = streamConfig.stream();
    if (allocator->allocate(stream) < 0) {
        std::cerr << "Failed to allocate buffers" << std::endl;
        return -1;
    }

    // Frame capture
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
            std::cerr << "Can't set buffer for request"
                << std::endl;
            return ret;
        }

        requests.push_back(std::move(request));
    }

    // Set callback for frame request completion
    camera->requestCompleted.connect(requestComplete);

    // Start camera and queue requests
    camera->start();
    for (std::unique_ptr<Request> &request : requests) {
        camera->queueRequest(request.get());
    }

    // Wait for 3 seconds
    std::this_thread::sleep_for(10000ms);

    // Stop camera and release resources
    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();
    return 0;
}