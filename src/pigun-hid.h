#include <stdint.h>

#ifndef PIGUN_HID
#define PIGUN_HID


// data container for the HID joystick report
typedef struct pigun_report_t pigun_report_t;
struct pigun_report_t {

	int16_t x;
	int16_t y;
	uint8_t buttons;
};


void* pigun_cycle(void*);


#endif
