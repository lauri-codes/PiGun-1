# PiGun-1
PiZero software for the PiGun model 1.

## Compile the code

Everything can be compiled with the makefile, running the following commands in the prompt:

```bash
make bluetooth
make pigun
```

The first command compiles the bluetooth stack required by the PiGun software, and usually takes some time.
The second command compiles the PiGun specific code and creates the executable `pigun.exe`
