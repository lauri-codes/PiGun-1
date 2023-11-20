#include <stdint.h>

#ifndef PIGUN_HID
#define PIGUN_HID


// data container for the HID joystick report
typedef struct pigun_report_t pigun_report_t;
struct pigun_report_t {

	short x;
	short y;
	uint8_t buttons;
};

extern pigun_report_t global_pigun_report;

typedef struct pigun_feedback_report_t pigun_feedback_report_t;
struct pigun_feedback_report_t {

	uint8_t  reportId;                                 // Report ID = 0x03 (3)
	
	// Collection: CA:GamePad CL:SetEffectReport - this is copied from xboxone_model_1914_bluetoothle
	uint8_t  PID_GamePadSetEffectReportDcEnableActuators : 4; // Usage 0x000F0097: DC Enable Actuators, Value = 0 to 1
	uint8_t  : 4;                                      // Pad
	uint8_t  PID_GamePadSetEffectReportMagnitude[4];   // Usage 0x000F0070: Magnitude, Value = 0 to 100
	uint8_t  PID_GamePadSetEffectReportDuration;       // Usage 0x000F0050: Duration, Value = 0 to 255, Physical = Value in 10⁻² s units
	uint8_t  PID_GamePadSetEffectReportStartDelay;     // Usage 0x000F00A7: Start Delay, Value = 0 to 255, Physical = Value in 10⁻² s units
	uint8_t  PID_GamePadSetEffectReportLoopCount;      // Usage 0x000F007C: Loop Count, Value = 0 to 255
};



#ifdef __cplusplus
extern "C" {
#endif
	
	void* pigun_cycle(void*);

#ifdef __cplusplus
}
#endif


#endif
