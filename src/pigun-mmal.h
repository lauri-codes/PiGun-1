#ifndef PIGUN_MMAL
#define PIGUN_MMAL


// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Camera acquisiton settings: This captures the full FoV with the V2.1 camera module
#define PIGUN_CAM_X 1280
#define PIGUN_CAM_Y 720
#define PIGUN_FPS 65

// Camera output settings: these are 4/10 of the camera acquisition. The
// vertical resolution needs to be a multiple of 16, and the horizontal
// resolution needs to be a multiple of 32!
#define PIGUN_RES_X 512
#define PIGUN_RES_Y 288

// total number of pixels in the buffer - has to be the product of the previous 2
#define PIGUN_NPX 147456


int pigun_mmal_init(void);



#endif
