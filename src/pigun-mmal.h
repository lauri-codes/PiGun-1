#ifndef PIGUN_MMAL
#define PIGUN_MMAL


// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Camera acquisiton settings
#define PIGUN_CAM_X 640
#define PIGUN_CAM_Y 480
#define PIGUN_FPS 120

// Camera output settings
#define PIGUN_RES_X 640
#define PIGUN_RES_Y 480

// total number of pixels in the buffer - has to be the product of the previous 2
#define PIGUN_NPX 307200


int pigun_mmal_init(void);



#endif
