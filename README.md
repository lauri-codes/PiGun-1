# PiGun-1

This is the software for PiGun (model 1), a DIY infra-red lightgun for modern systems. PiGun runs on a PiZero W with PiCamera V2.1 NoIR, and connects to the host computer via classic bluetooth.
It appears to the host as an HID joystick (2-axis, 8-buttons).

It was successfully tested with several games using MAME, Teknoparrot and Demulshooter.


## Before you begin

PiGun requires libmmal and libbcm2835, both libraries and headers (should be alread present on Raspbian, if not then apt-get them).

The repo inludes stl models for all hardware parts. These were designed to be made on a FDM 3D printer with a PLA filament.

There is also the PCB design in a zip file, ready for submission to manufacturers (JLPCB, Aisler, ...).
There is no design for the power supply board, but it is easy to make.

## Clone the repository and submodules

```bash
git clone https://github.com/fullmetalfelix/PiGun-1.git # use git@github.com:fullmetalfelix/PiGun-1 for SSH access
git submodule update --init --recursive
```

## Compiling the Code

After cloning the repo on the Raspberry Pi, navigate to the source code folder, typically `PiGun-1/src`.
Everything can be compiled with the makefile, running the following commands in the prompt:

```bash
make bluetooth
make pigun
```

The first command compiles the bluetooth stack required by the PiGun software, and usually takes some time.
The second command compiles the PiGun specific code and creates the executable `pigun.exe`

Ideally, PiGun should run at startup using a systemd unit file, so it will being shortly after the power cable is connected to the device.
When debugging it can just be started from `PiGun-1/src`, for example via SSH.


### GPIO Configuration

PiGun buttons, LEDS and the solenoid are connected to GPIO pins according to the definitions found in `pigun-gpio.h` header file, for example:
```C
#define PIN_TRG RPI_V2_GPIO_P1_15       # button pin definition
#define PIN_OUT_ERR RPI_V2_GPIO_P1_36   # LED (error) pin definition
#define PIN_OUT_SOL RPI_V2_GPIO_P1_07   # solenoid control pin
```
For convenience, the values are from macros defined in libbcm2835 for the Pi Zero pinout (RPI_V2_GPIO_P1_xx).
The ones defined in the source code match the PCB design. Make sure the definitions match the pins where buttons/LEDs are physically connected when using a custom-made board.

> [!WARNING]  
> Make sure the flags are the appropriate ones for the RPi model running PiGun and that they match the pins where buttons/LEDs are physically connected on a custom-made board.

Button pins are set as inputs with a pullup resistor, so they need to be grounded when pressed. LED and solenoid pins are set as output.


### Solenoid Configuration

The default solenoid setup assumes it is connected to the output of a NE555 timer, that generates a fixed time pulse when a recoil event is raised. In this case, a GPIO is connected to the 555 trigger (normally set to HIGH), which goes to LOW when the solenoid needs to be fired. This setup is defined in `pigun-gpio.h` as follows:
```C
#define SOL_FIRE 0
#define SOL_HOLD 1
```
`SOL_FIRE` is the value that the solenoid GPIO will take when the solenoid needs to fire, while `SOL_HOLD` is the value when not oprating. The two flags need to be logical opposite.

In a simpler setup, the solenoid is directly connected to a GPIO (normally set to LOW), and it is the PiGun code that generates the solenoid pulse directly by switching to HIGH the output pin for the desired duration of the pulse. If your setup is designed this way, these definitions should be inverted:
```C
#define SOL_FIRE 1
#define SOL_HOLD 0
```




## How it all works

The working principle of PiGun is similar to other IR lightguns: a camera in the lightgun sees IR beacons and calculates the aim-point coordinates based on their apparent positions.
PiGun requires four IR beacons, mounted so they form a rectangle, to be visible at all time. This makes the aiming calculation more robust with respect to changes in the player position relative to the screen.
Unlike common lightgun designs that behave like a mouse, PiGun is detected by the host computer as a bluetooth HID joystick (2-axis 8-buttons), so there is no problem having multiple PiGuns connected and working at the same time.

The software runs in two threads: the main one handles bluetooth communication, the second one manages the camera feed and aim-point calculation.
The bluetooth thread runs BTstack code, adapted from their examples. The camera thread initialised the PiCamera using MMAL and performs calculations on the acquired frames.

This is a rendering of the current PiGun (model 1) CAD model showing the buttons layout:

<img src="/3DModels/PG-1F/buttons.PNG" height="512"/>

There are 8 usable buttons that can be mapped in games and emulators, and a service mode button (CAL) on the right side of the lightgun that does not show up in the HID report (so it cannot be used by games).

The appearance is what it is. I modelled around a M92, but *adjustments* had to be made. 
It is quite a bit thicker to accommodate all the DIY electronic boards. The barrel is longer to house the PiZero and camera board.
The underbarrel extension is there for the board and buttons, which I could not yet make smaller.
The slide makes it look like a revolver, but it was the only way to fit a 24V (25N) solenoid and have a non-completely-pathetic recoil (improved with some magnets). Other seemigly odd design features are workarounds for the jankyness of this being 3D printed, or caused by lack of competence in the fields of design and engineering.
On the other hand, it can be made with available components.


### Connecting to a computer

After launching the software on the PiZero, just pair it from the host computer and it is good to go.
PiGun remembers the last 3 MAC addresses of the hosts it has been connected to, and it will try to reconnect to them at startup.
LED_OK blinks while PiGun is not connected to a host, and goes off as soon a connection is established.


### Calibration

PiGun needs to be calibrated to work properly, and it is done directly on the lightgun, without the use of any external software:

1. press CAL button - PiGun goes in service mode (LED_CAL turns on)
2. press TRG - PiGun is now in calibration mode
3. aim for the top-left corner of the play area and press TRG
4. aim for the bottom-right corner of the play area and press TRG
5. Pigun goes back to idle mode (LED_CAL turns off), ready to fire!

When aiming at the corners, it is important to ignore whatever crosshair might appear on screen (e.g. if you are running MAME) and aim with the PiGun sights.
Under the hood, the rectangle formed by the beacons provides a natural, normalised coordinate frame on the plane of the screen where the top-left corner is (0,0) and the bottom-right one is (1,1).
Shooting the actual corners sets the origin and scale of this frame of refence, defining the rectangle of the play area on the same plane. The PiGun x/y axis are then constrained to this rectangle.

The aim-point calculation is done with an inverse perspective transform, so technically it should not matter if the player is not in the same location where calibration happened.In practice it might a bit!

PiGun remembers the calibration settings so it should not be necessary to calibrate again unless the play area changes (e.g. different games with varying window size), or the beacons are moved relative to the play area.

> [!TIP]
> It is good practice to always calibrate an instrument before use.



### Recoil Mode
There are four operational modes for the recoil:

1. SELF: recoil is triggered with each trigger pull
2. AUTO: recoil fires at regular intervals while trigger is pulled
3. HID: recoil is fired by the host via specific HID output data reports
4. OFF: no recoil

The recoil mode is changed as follows:

1. press CAL button - PiGun goes in service mode (LED_CAL turns on)
2. press BT0 (D-pad central button) - switches to the next recoil mode
3. repeat step 2 until PiGun is in the desired mode
4. press CAL button - PiGun goes back to idle mode (LED_CAL turns off)

PiGun always starts in SELF recoil mode. When switching to next mode, if the current mode is OFF, PiGun will go back to SELF.
There is no visible feedback when changing mode, except switching back to SELF triggers one recoil event.

AUTO mode will fire the recoil repeatedly, but the HID report associated with the TRG button will only switch from 0 to 1 once, and stay 1 until TRG is released.

HID mode is meant to work with MAMEHooker on roms that expose the game outputs, so that the lightgun will only recoil when a shot is fired in the game.
These commands should be configures in MAMEHooker .ini files for your roms:
```
(when rom starts, e.g. MameStart)
ghd [joyID] &HA81 &H701 2 &h03:&h12

(when recoil fires, e.g. P1_CtmRecoil)
ghd [joyID] &HA81 &H701 2 &h03:&h01

(when rom ends, e.g. MameStop - optional)
ghd [joyID] &HA81 &H701 2 &h03:&h10
```
where `[joyID]` should be changed to the joystick ID of your PiGun (integer number). These are generic HID reports: the first two hex codes are the vendor and product ID of the PiGun, and the last two are the report ID and data (one byte each).
The first command sets PiGun in HID recoil mode (no need to do it manually via service mode), and the last one sets it back to SELF mode. The second command tells PiGun to recoil.

Unfortunately not all roms have outputs, even thought they should (Point Blank pls mamedevs!).


### Shutdown

One can always pull the plug, but the more civil way to shutdown PiGun (and the PiZero altogether) is to enter service mode and press and hold all the buttons on the lightgun handle in the right order:

1. press CAL button - PiGun goes in service mode (LED_CAL goes on)
2. press and hold RLD
3. press and hold MAG
4. press trigger - this will run `sudo shutdown now`



### IR considerations

Since PiCamera v2.1 NoIR is also sensitive to visible light, it is necessary to mount a band-pass filter that lets only IR light through. The beacon LEDs and filter should have matching IR wavelength.

It is also recommended to not have the gaming setup in front or near a window, since the sun is a very good IR beacon and will mess up the aim.

In principle the beacons should be as far from each other as possible, in order to get the highest aiming resolution. In practice, the camera has a limited field of view and it needs to see all four beacons when the lightgun is pointed at all corners of the play area.

The default setup works fine with just one LED per beacon, and the player ~1.5m away from them, but issues may arise on a different setup. For example, the detector code in `pigun-detector.c` processes the Y channel (intensity) of the camera feed, using a threshold value: pixels below threshold are considered background. The detector can fail if:

1. the IR LEDs in the beacons are not bright enough
2. the player is too far from them
3. the player is too close (camera FOV is not that wide, but wider than Gun 4 IR)
4. the player is not in front of the beacons (LEDs have a very narrow emission angle)

The LED_ERR will turn on if the detector routine ends without having found all the four IR spots in the camera feed. Each time PiGun enters service mode, the current camera frame is saved in the executable's folder. The file contains the Y channel, one byte for each pixel (416x320). This can be used to check that the beacons are working as intended (also doable with raspivid if the OS is running with X and PiZero is connected to a screen).

Possible solutions are:

1. more current through the IR LEDs, or mount more LEDs per beacon
2. same solution as for 1.
3. no solution for this one.
4. the Wii bar has multiple LEDs for each spot, and some are mounted at an angle, pointing inward and outward so that players do not have to compenetrate each other to play.

If multiple LEDs are mounted for one beacon, it is important that they are as close as possible to each other, in order to appear as a single symmetric light blob in the camera image, because the detector algorithm uses the *baricenter* of the spots for aim-point calculations.

Happy PiGunning!
