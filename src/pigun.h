
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


#include "btstack.h"

#include "pigun-hid.h"
#include "pigun-detector.h"


#ifndef PIGUN
#define PIGUN



extern pthread_mutex_t pigun_mutex;


typedef enum {
   STATE_IDLE = 0,
   STATE_SERVICE,
   STATE_CAL_TL,
   STATE_CAL_BR,

   STATE_SHUTDOWN
} pigun_state_t;

typedef enum {
   RECOIL_SELF=0,
   RECOIL_HID,
   RECOIL_OFF
}pigun_recoilmode_t;

/// @brief Represents a 2D point with f32 coordinates.
typedef struct {
	float x, y;
}pigun_aimpoint_t;



typedef struct {

   pigun_state_t state;

   pigun_recoilmode_t recoilMode;
   int8_t recoilCooldownTimer;

   uint8_t autofire;
   uint8_t button_rest;


   // *** DETECTION SYSTEM ***
   // stores the current camera frame
   unsigned char     *framedata;
   pigun_detector_t  detector;


   // *** AIMING CALCULATOR ***
   // normalised aiming point - before calibration applies
   pigun_aimpoint_t  aim_normalised;

   // calibration points in normalised frame of reference
   pigun_aimpoint_t  cal_topleft;
   pigun_aimpoint_t  cal_lowright;


   // *** CONNECTIVITY ***
   uint8_t           nServers;   // number of past servers stored in the file - up to 3
   bd_addr_t         servers[3]; // server addresses - up to 3





   pigun_report_t    report;


   


}pigun_object_t;
extern pigun_object_t pigun;



void pigun_calibration_save(void);


// these function define how aiming works
void pigun_calculate_aim();


// HELPER FUNCTIONS
int pigun_camera_gains(MMAL_COMPONENT_T *camera, int analog_gain, int digital_gain);
int pigun_camera_awb(MMAL_COMPONENT_T *camera, int on);
int pigun_camera_awb_gains(MMAL_COMPONENT_T *camera, float r_gain, float b_gain);
int pigun_camera_blur(MMAL_COMPONENT_T *camera, int on);
int pigun_camera_exposuremode(MMAL_COMPONENT_T *camera, int on);


#endif
