/*
Here are all the PiGun functions related to camera stuff with libmmal.
*/

#include "pigun.h"
#include "pigun-detector.h"
#include "pigun-mmal.h"
#include "pigun-gpio.h"

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"


MMAL_PORT_T* pigun_video_port;
MMAL_POOL_T* pigun_video_port_pool;


void video_buffer_release(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {

	// we are done with this buffer, we can release it!
	mmal_buffer_header_release(buffer);

	// and send one back to the port (if still open)
	if (port->is_enabled) {

		MMAL_STATUS_T status = MMAL_SUCCESS;
		MMAL_BUFFER_HEADER_T* new_buffer;
		new_buffer = mmal_queue_get(pool->queue);

		if (new_buffer) status = mmal_port_send_buffer(port, new_buffer);

		if (!new_buffer || status != MMAL_SUCCESS)
			printf("PIGUN ERROR: Unable to return a buffer to the video port\n");
	}
}


/// @brief Called each time a camera frame is ready for processing.
/// buffer->data has the pixel values in the chosen encoding (I420).
/// @param port MMAL port object.
/// @param buffer Camera frame buffer object.
static void video_buffer_callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {

	MMAL_POOL_T* pool = (MMAL_POOL_T*)port->userdata;

	pigun.framedata = buffer->data;

	if(pigun.state == STATE_SHUTDOWN){
		video_buffer_release(port, buffer);
		return;
	}

    // call the peak detector function *************************************
	// if there was a detector error, error LED goes on, otherwise off
	// the switch only happens when the detector return value changes
	uint8_t ce = pigun.detector.error;
	pigun_detector_run(pigun.framedata);
	if(pigun.detector.error != ce) {
		// if the error flag changed, flip the LED state
		pigun_GPIO_output_set(PIN_OUT_ERR, pigun.detector.error);
	}

	// the peaks are supposed to be ordered by the detector function

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

	// we are done with this buffer, we can release it!
	video_buffer_release(port, buffer);
}



/// @brief Initialises the camera with libMMAL.
/// @return 0 if everything went fine.
int pigun_mmal_init() {

	printf("PIGUN: initializing camera...\n");

	MMAL_COMPONENT_T* camera = NULL;
	MMAL_COMPONENT_T* preview = NULL;

	MMAL_ES_FORMAT_T* format = NULL;

	MMAL_PORT_T* camera_preview_port = NULL;
	MMAL_PORT_T* camera_video_port = NULL;
	MMAL_PORT_T* camera_still_port = NULL;

	MMAL_POOL_T* camera_video_port_pool;

	MMAL_CONNECTION_T* camera_preview_connection = NULL;

	MMAL_STATUS_T status;

	// I guess this starts the driver?
	bcm_host_init();
	printf("PIGUN: BCM Host initialized.\n");

	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
	if (status != MMAL_SUCCESS) {
		printf("PIGUN ERROR: create camera returned %x\n", status);
		return -1;
	}

	// connect ports
	camera_preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
	camera_video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
	camera_still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

	// configure the camera component **********************************
	{
		MMAL_PARAMETER_CAMERA_CONFIG_T cam_config = {
			{ MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config)},
			.max_stills_w = PIGUN_CAM_X,
			.max_stills_h = PIGUN_CAM_Y,
			.stills_yuv422 = 0,
			.one_shot_stills = 1,
			.max_preview_video_w = PIGUN_CAM_X,
			.max_preview_video_h = PIGUN_CAM_Y,
			.num_preview_video_frames = 3,
			.stills_capture_circular_buffer_height = 0,
			.fast_preview_resume = 0,
			.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
		};
		mmal_port_parameter_set(camera->control, &cam_config.hdr);
	}
	// *****************************************************************
	// Setup camera video port format **********************************
	format = camera_video_port->format;

	format->encoding = MMAL_ENCODING_I420;
	format->encoding_variant = MMAL_ENCODING_I420;

	// video port outputs reduced resolution
	format->es->video.width = PIGUN_RES_X;
	format->es->video.height = PIGUN_RES_Y;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = PIGUN_RES_X;
	format->es->video.crop.height = PIGUN_RES_Y;
	format->es->video.frame_rate.num = PIGUN_FPS;
	format->es->video.frame_rate.den = 1;

	camera_video_port->buffer_size = camera_video_port->buffer_size_recommended;
	camera_video_port->buffer_num = 4;

	// apply the format
	status = mmal_port_format_commit(camera_video_port);
	if (status != MMAL_SUCCESS) {
		printf("PIGUN ERROR: unable to commit camera video port format (%u)\n", status);
		return -1;
	}

	printf("PIGUN: camera video buffer_size = %d\n", camera_video_port->buffer_size);
	printf("PIGUN: camera video buffer_num = %d\n", camera_video_port->buffer_num);
	// *****************************************************************
	// crate buffer pool from camera.video output port *****************
	camera_video_port_pool = (MMAL_POOL_T*)mmal_port_pool_create(
		camera_video_port,
		camera_video_port->buffer_num,
		camera_video_port->buffer_size
	);
	camera_video_port->userdata = (struct MMAL_PORT_USERDATA_T*)camera_video_port_pool;

	// the port is enabled with the given callback function
	// the callback is called when a complete frame is ready at the camera.video output port
	status = mmal_port_enable(camera_video_port, video_buffer_callback);
	if (status != MMAL_SUCCESS) {
		printf("PIGUN ERROR: unable to enable camera video port (%u)\n", status);
		return -1;
	}
	printf("PIGUN: frame buffer created.\n");
	// *****************************************************************

	// not sure if this is needed - seems to work without as well
	status = mmal_component_enable(camera);

	printf("PIGUN: setting up parameters\n");
	
	// Disable exposure mode
	pigun_camera_exposuremode(camera, 0);

	// Set gains
	//if (argc == 3) {
		//int analog_gain = atoi(argv[1]);
		//int digital_gain = atoi(argv[2]);
		//pigun_camera_gains(camera, analog_gain, digital_gain);
	//}
	//
	// Setup automatic white balance
	//pigun_camera_awb(camera, 0);
	//pigun_camera_awb_gains(camera, 1, 1);

	// Setup blur
	pigun_camera_blur(camera, 1);
	printf("PIGUN: parameters set\n");

	// send the buffers to the camera.video output port so it can start filling them frame data

	// Send all the buffers to the encoder output port
	int num = mmal_queue_length(camera_video_port_pool->queue);
	int q;
	for (q = 0; q < num; q++) {
		MMAL_BUFFER_HEADER_T* buffer = mmal_queue_get(camera_video_port_pool->queue);

		if (!buffer)
			printf("PIGUN ERROR: Unable to get a required buffer %d from pool queue\n", q);

		if (mmal_port_send_buffer(camera_video_port, buffer) != MMAL_SUCCESS)
			printf("PIGUN ERROR: Unable to send a buffer to encoder output port (%d)\n", q);

		// we are not really dealing with errors... they are not supposed to happen anyway!
	}

	status = mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1);
	if (status != MMAL_SUCCESS) {
		printf("PIGUN ERROR: %s: Failed to start capture\n", __func__); // what is this func?
		return -1;
	}

	// save necessary stuff to global vars
	pigun_video_port = camera_video_port;
	pigun_video_port_pool = camera_video_port_pool;

	printf("PIGUN: camera initialised.\n");
	return 0;
}

