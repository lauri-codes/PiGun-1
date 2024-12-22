#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include <libcamera/libcamera.h>

using namespace libcamera;
using namespace std::chrono_literals;

static std::shared_ptr<Camera> camera;

int main()
{
    // Create and start the CameraManager
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    // Acquire the camera
    auto cameras = cm->cameras();
    if (cameras.empty()) {
        std::cout << "No cameras were identified on the system."
                << std::endl;
        cm->stop();
        return EXIT_FAILURE;
    }
    std::string cameraId = cameras[0]->id();
    auto camera = cm->get(cameraId);
    camera->acquire();

    // Configure camera
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::Viewfinder } );
    StreamConfiguration &streamConfig = config->at(0);
    streamConfig.size.width = 640; 
    streamConfig.size.height = 480;
    streamConfig.pixelFormat = formats::YUV420;
    CameraConfiguration::Status status = config->validate();
    if (status == CameraConfiguration::Status::Invalid) {
        std::cerr << "Configuration invalid" << std::endl;
        return -1;
    }

    // Stop camera and release resources
    camera->stop();
    camera->release();
    camera.reset();
    cm->stop();
    return 0;
}