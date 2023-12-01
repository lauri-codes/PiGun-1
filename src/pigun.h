
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <sysexits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include <semaphore.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>


#ifndef PIGUN
#define PIGUN


// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Camera acquisiton settings: This captures the full FoV with the V2 camera
// module
#define PIGUN_CAM_X 1640
#define PIGUN_CAM_Y 1232
#define PIGUN_FPS 40

// Camera output settings: these are ~1/4th of the camera acquisition. The
// vertical resolution needs to be a multiple of 16, and the horizontal
// resolution needs to be a multiple of 32!
#define PIGUN_RES_X 416 // 410 // 320
#define PIGUN_RES_Y 320 // 308 // 180

// total number of pixels in the buffer - has to be the product of the previous 2
#define PIGUN_NPX 133120 // 126280 // 57600 // 230400

#define PI 3.14159265
extern MMAL_PORT_T *port_prv_in1;


// maximum distance in pixels for peak matching
#define MAXPEAKDIST 5

extern pthread_mutex_t pigun_mutex;


typedef enum {
   STATE_IDLE = 0,
   STATE_SERVICE,
   STATE_CAL_TL,
   STATE_CAL_BR,

   STATE_SHUTDOWN
} pigun_state_t;

typedef enum {
   SelfActivated=0,
   HIDdriven,
   Off,
}pigun_recoilmode_t;

/// @brief Represents a 2D point with f32 coordinates.
typedef struct {
	float x, y;
}pigun_aimpoint_t;

/// @brief Describes a peak in the camera image.
typedef struct {
   float row;
   float col;
   float maxI;
   float total;
   float tRow, tCol;
   // 0->struct is unused, 1->peak found
   uint16_t found;
} pigun_peak_t;

typedef struct {

   pigun_state_t state;

   uint8_t autofire;
   uint8_t button_rest;
   

   // normalised aiming point - before calibration applies
   pigun_aimpoint_t aim_normalised;

   // calibration points in normalised frame of reference
   pigun_aimpoint_t cal_topleft;
   pigun_aimpoint_t cal_lowright;

   pigun_peak_t *peaks;

   pigun_report_t report;




}pigun_object_t;
extern pigun_object_t pigun;



extern unsigned char* pigun_framedata;




void pigun_calibration_save(void);


// these function define how detection and aiming works
int pigun_detect(unsigned char* data);
void pigun_calculate_aim();


// HELPER FUNCTIONS
int pigun_camera_gains(MMAL_COMPONENT_T *camera, int analog_gain, int digital_gain);
int pigun_camera_awb(MMAL_COMPONENT_T *camera, int on);
int pigun_camera_awb_gains(MMAL_COMPONENT_T *camera, float r_gain, float b_gain);
int pigun_camera_blur(MMAL_COMPONENT_T *camera, int on);
int pigun_camera_exposuremode(MMAL_COMPONENT_T *camera, int on);


#endif
