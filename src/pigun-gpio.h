//#include <bcm2835.h>
#include <pigpio.h>

#include <stdint.h>
#include "pigun-hid.h"


#ifndef PIGUN_GPIO
#define PIGUN_GPIO


// Button definitions - GPIO pins
#define PIN_TRG RPI_V2_GPIO_P1_11	// trigger 
#define PIN_RLD RPI_V2_GPIO_P1_13	// reload  (this is the little clip button like in real beretta M9)
#define PIN_MAG RPI_V2_GPIO_P1_15	// handmag (this should be connected under the handle)  --  seems to be soldered ok!

#define PIN_BT0 RPI_V2_GPIO_P1_18	// central button in the d-pad
#define PIN_BTU RPI_V2_GPIO_P1_24	// d-pad buttons
#define PIN_BTD RPI_V2_GPIO_P1_32
#define PIN_BTL RPI_V2_GPIO_P1_22
#define PIN_BTR RPI_V2_GPIO_P1_16


#define PIN_CAL RPI_V2_GPIO_P1_10	// calibration button


#define MASK_TRG UINT16_C(0x0001)
#define MASK_RLD UINT16_C(0x0002)
#define MASK_MAG UINT16_C(0x0004)

#define MASK_BT0 UINT16_C(0x0008)


#define MASK_CAL UINT16_C(0x0100)

//extern uint16_t pigun_button_newpress;


/* GPIO for LEDs
* 
* LED behaviour:
*
*	green LED (PIN_OUT_AOK) blinks when the bluetooth is searching for connection
*	and it will turn off once connected to host to save power
*
*	yellow LED (PIN_OUT_CAL) turns on when in calibration mode
*
*	red LED (PIN_OUT_ERR) turns on when the program cannot detect 4 peaks in the camera image
*
*/
#define PIN_OUT_ERR RPI_V2_GPIO_P1_36
#define PIN_OUT_CAL RPI_V2_GPIO_P1_38
#define PIN_OUT_AOK RPI_V2_GPIO_P1_40

// GPIO that triggers the solenoid
#define PIN_OUT_SOL RPI_V2_GPIO_P1_08

// these are the pin states for fire and not-fire
#define SOL_FIRE 0 // set to 0 if the SOL pin drives the 555 trigger, set to 1 if the SOL pin drives the optocoupler directly
#define SOL_HOLD 1 // set this to opposite of SOL_FIRE




// functions
int pigun_GPIO_init();
int pigun_GPIO_stop();

void pigun_GPIO_output_set(int LEDPIN, int state);
void pigun_recoil_fire(void);
void pigun_buttons_process(void);


#endif
