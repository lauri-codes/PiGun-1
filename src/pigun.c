 /* This code sets up the camera and start the acquision/aiming loop.
 
 GENERAL IDEA OF THIS CODE
  *
  * the camera acquires... as fast as possible i guess!
  * camera.video output is configured with half resolution of the camera acquisition, at 90 fps
  * camera.video output is NOT connected to anything, but buffered into a pool
  * camera.video -> buffer (video_buffer_callback)
  * camera
  * video_buffer_callback copies the Y channel into another buffer
  * we use the Y channel to detect the IR LEDs
  *
  * camera video output uses MMAL_ENCODING_I420
  *
  *
  * */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include <bcm2835.h>


#include "pigun.h"
#include "pigun-gpio.h"
#include "pigun-hid.h"
#include "pigun-mmal.h"
#include "pigun-detector.h"


#include <math.h>
#include <stdint.h>
#include <signal.h>


// main object with all pigun info
pigun_object_t pigun;

pthread_mutex_t pigun_mutex;


/// @brief Save the calibration data for future use.
void pigun_calibration_save(){
	
	FILE* fbin = fopen("cdata.bin", "wb");
	fwrite(&(pigun.cal_topleft),  sizeof(pigun_aimpoint_t), 1, fbin);
	fwrite(&(pigun.cal_lowright), sizeof(pigun_aimpoint_t), 1, fbin);
	fclose(fbin);
}


static void preview_buffer_callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) { mmal_buffer_header_release(buffer); }




//void * test_main(int argc, char** argv) {
void* pigun_cycle(void* nullargs) {

	pigun.state = STATE_IDLE;
	pigun_detector_init();

	// reset calibration
	pigun.cal_topleft.x = pigun.cal_topleft.y = 0;
	pigun.cal_lowright.x = pigun.cal_lowright.y = 1;
	
	// load calibration data if available
	FILE* fbin = fopen("cdata.bin", "rb");
	if (fbin == NULL) printf("PIGUN: no calibration data found\n");
	else {
		fread(&(pigun.cal_topleft), sizeof(pigun_aimpoint_t), 1, fbin);
		fread(&(pigun.cal_lowright), sizeof(pigun_aimpoint_t), 1, fbin);
		fclose(fbin);
	}

	// allocate peaks - 10 to be sure there is enough space
	pigun.peaks = (pigun_peak_t*)calloc(10, sizeof(pigun_peak_t));

	// pins should be initialised using the function in the GPIO module
	// called by the main thread when the program starts
	// because the bluetooth (HID) part also uses the LEDs to inform about connection status
	
	// Initialize the camera system
	int error = pigun_mmal_init();
	if (error != 0) {
		pigun_GPIO_output_set(PIN_OUT_ERR, 1);
		return NULL;
	}
	printf("PIGUN: MMAL started correctly.\n");
	
	// repeat forever and ever!
	// there could be a graceful shutdown?
	int cameraON;
	while (1) {
		
		cameraON = 1;
		switch (pthread_mutex_trylock(&pigun_mutex)) {
		case 0: /* if we got the lock, unlock and return 1 (true) */
			pthread_mutex_unlock(&pigun_mutex);
			cameraON = 1;
			break;
		case EBUSY: /* return 0 (false) if the mutex was locked */
			cameraON = 0;
			break;
		}
		
		// if this thread could get the mutex lock, then the main thread is signalling a stop!
		if (cameraON) break;
	}
	
	free(pigun.peaks);

	pthread_exit((void*)0);
}

