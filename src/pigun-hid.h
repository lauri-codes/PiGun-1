#include <stdint.h>

#ifndef PIGUN_HID
#define PIGUN_HID


#define PIGUN_REPORT_ID 0x03


// data container for the HID joystick report
typedef struct pigun_report_t pigun_report_t;
struct pigun_report_t {

	int16_t x;
	int16_t y;
	uint8_t buttons;
};

typedef void (*blinker_callback_t)(btstack_timer_source_t *ts);

typedef struct {

	uint8_t 	active;
	uint8_t 	nblinks;	// 0=infinite blinks
	uint8_t 	counter;
	uint16_t 	timeout; 	// in ms

	btstack_timer_source_t timer;
	blinker_callback_t callback;

} pigun_blinker_t;



void* pigun_cycle(void*);


#endif
