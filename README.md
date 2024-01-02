# PiGun-1
PiZero software for the PiGun model 1.

## Compile the Code

Everything can be compiled with the makefile, running the following commands in the prompt:

```bash
make bluetooth
make pigun
```

The first command compiles the bluetooth stack required by the PiGun software, and usually takes some time.
The second command compiles the PiGun specific code and creates the executable `pigun.exe`

## GPIO Configuration

PiGun buttons, LEDS and the solenoid are connected to GPIO pins according to the definitions found in `pigun-gpio.h` header file, for example:
```C
#define PIN_TRG RPI_V2_GPIO_P1_15       # button pin definition
#define PIN_OUT_ERR RPI_V2_GPIO_P1_36   # LED (error) pin definition
#define PIN_OUT_SOL RPI_V2_GPIO_P1_07   # solenoid control pin
```
For convenience, the values are from macros defined by libbcm2835, and they might need to be different if the buttons/LEDs are not physically connected to the default pins.
Make sure the flags correspond to your pins and to the RPi type where PiGun is running.

Button pins are set as inputs with a pullup resistor, so they need to be grounded when pressed. LED and solenoid pins are set as output.


## Solenoid Configuration

The default solenoid setup assumes it is connected to the output of a NE555 timer, that generates a fixed time pulse when the trigger is pressed. In this case, a GPIO is connected to the 555 trigger and is set to logic LOW when the solenoid needs to be fired up. This setup is defined in `pigun-gpio.h` as follows:
```C
#define SOL_FIRE 0
#define SOL_HOLD 1
```
`SOL_FIRE` is the value that the solenoid GPIO will take when the solenoid needs to fire, while `SOL_HOLD` is the rest value. The two need to be logical opposite.

A simpler setup generates the solenoid pulse directly in the PiGun code by switching on the output pin for the desired duration of the pulse. In this mode, the definitions should be inverted:
```C
#define SOL_FIRE 1
#define SOL_HOLD 0
```

## Service Mode

Press the service mode button (connected to pin PIN_CAL) to enter the PiGun service mode, and change its settings. Service mode is a state machine following this schema:

| State | TRG | RLD | MAG | BT0 |
| :---: | :---: | :---: | :---: | :---: |
| IDLE | shoot | button press | button press | button press |



