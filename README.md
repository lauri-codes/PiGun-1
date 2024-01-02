# PiGun-1

This is the software for PiGun (model 1), a DIY infra-red lightgun for modern systems. PiGun runs on a PiZero W with PiCamera V2.1 NoIR, and connects to the host computer via classic bluetooth.
It appears to the host as an HID joystick (2-axis, 8-buttons).


This is a rendering of the current PiGun model 1 CAD model:

<img src="/3DModels/PG-1F/buttons.PNG" height="512"/>






## Compile the Code

Everything can be compiled with the makefile, running the following commands in the prompt:

```bash
make bluetooth
make pigun
```

The first command compiles the bluetooth stack required by the PiGun software, and usually takes some time.
The second command compiles the PiGun specific code and creates the executable `pigun.exe`

The PiGun executable can be run at startup using a systemd unit file, so it will being shortly after the power cable is connected to the device.


### GPIO Configuration

PiGun buttons, LEDS and the solenoid are connected to GPIO pins according to the definitions found in `pigun-gpio.h` header file, for example:
```C
#define PIN_TRG RPI_V2_GPIO_P1_15       # button pin definition
#define PIN_OUT_ERR RPI_V2_GPIO_P1_36   # LED (error) pin definition
#define PIN_OUT_SOL RPI_V2_GPIO_P1_07   # solenoid control pin
```
For convenience, the values are from macros defined by libbcm2835, and they might need to be different if the buttons/LEDs are not physically connected to the default pins.
Make sure the flags correspond to your pins and to the RPi type where PiGun is running.

Button pins are set as inputs with a pullup resistor, so they need to be grounded when pressed. LED and solenoid pins are set as output.


### Solenoid Configuration

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


## How it all works

The working principle of PiGun is similar to other IR lightguns: a camera in the lightgun sees IR beacons and calculated the aim-point coordinates based on their apparent positions.
Unlike other IR-based solutions, PiGun requires four IR beacons, mounted so they form a rectangle, to be visible at all time. This makes the aiming calculation more robust with respect to changes in the player position relative to the screen.
More importantly, PiGun is detected by the host computer as a bluetooth HID joystick (2-axis 8-buttons), so there is no problem having multiple PiGuns connected and working at the same time. 

The software runs in two threads: the main one handles bluetooth communication, the second one manages the camera feed and aim-point calculation.


### Connecting to a computer
After launching the software on the PiZero, just pair it from the host computer and it is good to go.
PiGun remembers the last 3 MAC addresses of the hosts it has been connected to, and it will try to reconnect to them at startup.


### Calibration
PiGun needs to be calibrated to work properly, and it is done directly on the lightgun, without the use of any external software:

1. press the service mode button (connected to pin PIN_CAL) - PiGun goes in service mode (LED_CAL goes on)
2. press the trigger - PiGun is now in calibration mode
3. shoot the top-left corner of the play area
4. shoot the bottom-right corner of the play area
5. press the service mode button - PiGun goes back to idle mode (LED_CAL goes off)

When aiming at the corners, it is important to ignore whatever crosshair might appear on screen (e.g. if you are running MAME) and aim with the PiGun sights.
What happens under the hood is simple. The rectangle formed by the beacons provides a natural, normalised coordinate frame on a plane, where the top-left corner is (0,0) and the bottom-right one is (1,1). Shooting the actual corners sets the origin and scale of this frame of refence, defining the rectangle of the play area on the same plane. The PiGun x/y axis are then constrained to this rectangle.

With the 4-beacons setup, the aim-point calculation can be done with an inverse perspective transform, so technically it should not matter if the player is not in the same location where calibration happened. In practice it might a bit!

PiGun remembers the calibration settings so it should not be necessary to calibrate again unless the play area changes (e.g. different games with varying window size), or the beacons are moved relative to the play area.
However, every good physicist always calibrates their instruments before use!


### Recoil Mode
There are four operation modes for the recoil:

1. SELF: recoil is triggered with each trigger pull
2. AUTO: recoil fires at regular intervals while trigger is pulled
3. HID: recoil is fired by the host via specific HID output data reports
4. OFF: no recoil

The recoil mode is changed as follows:

1. press the service mode button (connected to PIN_CAL) - PiGun goes in service mode (LED_CAL goes on)
2. press the D-pad central button (connected to PIN_BT0) - switches to the next recoil mode
3. repeat 2 until PiGun is in the desired mode
4. press the service mode button - PiGun goes back to idle mode (LED_CAL goes off)

PiGun always starts in SELF recoil mode. When switching to next mode, if the current mode is OFF, PiGun will go back to SELF.
There is no visible feedback when changing mode, except switching back to SELF triggers one recoil event.

HID mode is meant to work with MAMEHooker on roms that expose outputs, so that the lightgun will only recoil when a shot is fired in the game.
These commands should be configures in MAMEHooker .ini files for your roms:
```
(when rom starts, e.g. MameStart)
ghd joyID &HA81 &H701 2 &h03:&h12

(when recoil fires, e.g. P1_CtmRecoil)
ghd joyID &HA81 &H701 2 &h03:&h01

(when rom ends, e.g. MameStop - optional)
ghd joyID &HA81 &H701 2 &h03:&h10
```
where joyID should be changed to the joystick ID of your PiGun (integer number). These are generic HID reports: the first two hex codes are the vendor and product ID of the PiGun, and the last two are the report ID and data.
The first command sets PiGun in HID recoil mode (no need to do it manually via service mode), and the last one sets it back to SELF mode. The second command tells PiGun to recoil.

Unfortunately not all roms have outputs, even thought they should (Point Blank pls mamedevs!).

### Shutdown
Once can always pull the plug, but the more civil way to shutdown PiGun (and the PiZero altogether) is to enter service mode and press and hold all the buttons on the lightgun handle in the right order:

1. press the service mode button (connected to PIN_CAL) - PiGun goes in service mode (LED_CAL goes on)
2. press and hold RLD
3. press and hold MAG
4. press trigger - this will run `sudo shutdown now`



### IR considerations
Since PiCamera v2.1 NoIR is also sensitive to visible light, it is necessary to mount a band-pass filter that lets only IR light through. The beacon LEDs and filter should have matching IR wavelength.
It is also recommended to not have the gaming setup in front or near a window, since sunlight can be detected in place of an IR beacon and mess up the aim.


