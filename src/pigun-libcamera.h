#ifndef PIGUN_LIBCAMERA
#define PIGUN_LIBCAMERA

// Camera output settings
#define PIGUN_RES_X 640
#define PIGUN_RES_Y 480
#define PIGUN_FPS 120

// total number of pixels in the buffer - has to be the product of the previous 2
#define PIGUN_NPX 307200

int pigun_libcamera_init(void);

#endif
