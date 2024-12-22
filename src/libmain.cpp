#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/camera.h>
#include <libcamera/request.h>

libcamera::CameraManager manager;
manager.start();

auto cameras = manager.cameras(); 
if (cameras.size() == 0) {
    std::cerr << "No cameras available\n";
    return -1;
}
// Typically there's only one camera on a Pi
std::shared_ptr<libcamera::Camera> camera = cameras[0];

if (camera->acquire()) {
    std::cerr << "Failed to acquire camera\n";
    return -1;
}

libcamera::CameraConfiguration *config = camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });
if (!config || config->size() == 0) {
    std::cerr << "Failed to generate camera configuration\n";
    return -1;
}

// Example: adjust the image size if you want
libcamera::StreamConfiguration &streamConfig = config->at(0);
// For instance, set to 640×480
streamConfig.size.width = 640;
streamConfig.size.height = 480;

// Validate the configuration
libcamera::CameraConfiguration::Status validation = config->validate();
if (validation == libcamera::CameraConfiguration::Status::Invalid) {
    std::cerr << "Configuration invalid\n";
    return -1;
}

// Apply the configuration
if (camera->configure(config) < 0) {
    std::cerr << "Failed to configure camera\n";
    return -1;
}

auto allocator = std::make_unique<libcamera::FrameBufferAllocator>(camera);
libcamera::Stream *stream = config->at(0).stream();
if (allocator->allocate(stream) < 0) {
    std::cerr << "Failed to allocate buffers\n";
    return -1;
}

// Store allocated buffers
std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = allocator->buffers(stream);

for (auto &buffer : buffers) {
    std::unique_ptr<libcamera::Request> request = camera->createRequest();
    if (!request) {
        std::cerr << "Failed to create request\n";
        return -1;
    }
    if (request->addBuffer(stream, buffer.get()) < 0) {
        std::cerr << "Failed to add buffer to request\n";
        return -1;
    }
    // Queue the request for capture
    camera->queueRequest(request.get());
    // Usually store the request in a queue or map
}

if (camera->start()) {
    std::cerr << "Failed to start camera\n";
    return -1;
}

camera->stop();
camera->release();
manager.stop();

