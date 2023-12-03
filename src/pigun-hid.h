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

typedef struct pigun_blinker_t pigun_blinker_t;
typedef void (*blinker_callback_t)(void);

struct pigun_blinker_t {

	uint8_t 	active;
	uint8_t 	nblinks;	// 0=infinite blinks
	uint8_t 	counter;
	uint8_t		cancelled;	// turn to 1 when pigun_blinker_stop is called
	uint16_t 	timeout; 	// in ms

	btstack_timer_source_t timer;
	blinker_callback_t callback;

};



void* pigun_cycle(void*);


#endif
