/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>


#include "btstack.h"

//#include <bcm2835.h>
#include "pigun-gpio.h" // this is mine!
#include "pigun-hid.h" // this is mine!



// from USB HID Specification 1.1, Appendix B.1
// this is custom made joystick with 8 buttons, and two 16-bit axis
/*
const uint8_t hid_descriptor_joystick_mode[] = {
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x04,        // Usage (Joystick)
	0xA1, 0x01,        // Collection (Application)
	0x16, 0x01, 0x80,  //   Logical Minimum 0x8001 (-32767)  
	0x26, 0xFF, 0x7F,  //   Logical Maximum 0x7FFF (32767)
	0x09, 0x01,        //   Usage (Pointer)
	0xA1, 0x00,        //   Collection (Physical)
	0x09, 0x30,        //     Usage (X)
	0x09, 0x31,        //     Usage (Y)
	0x75, 0x10,        //     Report Size (16) --- 22 bytes
	0x95, 0x02,        //     Report Count (2)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection   --- 27 bytes
	0x05, 0x09,        //   Usage Page (Button)
	0x19, 0x01,        //   Usage Minimum (0x01)
	0x29, 0x08,        //   Usage Maximum (0x08)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x01,        //   Logical Maximum (1) --- 37 bytes
	0x75, 0x01,        //   Report Size (1)
	0x95, 0x08,        //   Report Count (8)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              // End Collection --- 44 bytes
};
*/
const uint8_t hid_descriptor_joystick_mode[] = {
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x04,        // Usage (Joystick)
	0xA1, 0x01,        // Collection (Application)
	0x85, 0x01,        //   Report_ID (1)
	0x09, 0x01,        //   Usage (Pointer)
	0xA1, 0x00,        //   Collection (Physical)
	0x09, 0x30,        //     Usage (X)
	0x09, 0x31,        //     Usage (Y)
	0x16, 0x01, 0x80,  //     Logical Minimum 0x8001 (-32767)  
	0x26, 0xFF, 0x7F,  //     Logical Maximum 0x7FFF (32767)
	0x95, 0x02,        //     Report Count (2)
	0x75, 0x10,        //     Report Size (16)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection (Physical)
	0x05, 0x09,        //   Usage Page (Button)
	0x19, 0x01,        //   Usage Minimum (0x01)
	0x29, 0x08,        //   Usage Maximum (0x08)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x01,        //   Logical Maximum (1)
	0x75, 0x01,        //   Report Size (1)
	0x95, 0x08,        //   Report Count (8)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	// 45 bytes so far!
	// now begins the new physical interface device for feedback

	0x85, 0x01,        //   Report_ID (1)
	0x95, 0x05,                    //   REPORT_COUNT (5)
	0x75, 0x01,                    //   REPORT_SIZE (1)
	0x05, 0x08,                    //   USAGE_PAGE (LEDs)
	0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
	0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
	0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x03,                    //   REPORT_SIZE (3)
	0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)

/*
  0x06, 0x00, 0xFF,            // (GLOBAL) USAGE_PAGE         0xFF00 Vendor-defined 
  0xA1, 0x01,                  // (MAIN)   COLLECTION         0x01 Application (Usage=0x0: Page=, Usage=, Type=) <-- Warning: USAGE type should be CA (Application)
  0x85, 0x02,                  //   (GLOBAL) REPORT_ID          0x02 (2) 
  0x09, 0x01,                  //   (LOCAL)  USAGE              0xFF000001  
  0x15, 0x00,                  //   (GLOBAL) LOGICAL_MINIMUM    0x00 (0) <-- Redundant: LOGICAL_MINIMUM is already 0
  0x26, 0xFF, 0x00,            //   (GLOBAL) LOGICAL_MAXIMUM    0x00FF (255) 
  0x75, 0x08,                  //   (GLOBAL) REPORT_SIZE        0x08 (8) Number of bits per field 
  0x95, 0x40,                  //   (GLOBAL) REPORT_COUNT       0x40 (1) Number of fields 
  0x09, 0x01,                  //   (LOCAL)  USAGE              0xFF000001  
  0x91, 0x02,                  //   (MAIN)   OUTPUT             0x00000002 (64 fields x 8 bits) 0=Data 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap 
  0xC0,                        // (MAIN)   END_COLLECTION     Application
*/
/*
0x05, 0x0F,        // Usage Page (PID Page)
0x09, 0x92,        // Usage (0x92)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x02,        //   Report ID (2)
0x09, 0x9F,        //   Usage (0x9F)
0x09, 0xA0,        //   Usage (0xA0)
0x09, 0xA4,        //   Usage (0xA4)
0x09, 0xA5,        //   Usage (0xA5)
0x09, 0xA6,        //   Usage (0xA6)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x35, 0x00,        //   Physical Minimum (0)
0x45, 0x01,        //   Physical Maximum (1)
0x75, 0x01,        //   Report Size (1)
0x95, 0x05,        //   Report Count (5)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x03,        //   Report Count (3)
0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x94,        //   Usage (0x94)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x35, 0x00,        //   Physical Minimum (0)
0x45, 0x01,        //   Physical Maximum (1)
0x75, 0x01,        //   Report Size (1)
0x95, 0x01,        //   Report Count (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x07,        //   Report Size (7)
0x95, 0x01,        //   Report Count (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x09, 0x21,        // Usage (0x21)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x01,        //   Report ID (1)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x25,        //   Usage (0x25)
0xA1, 0x02,        //   Collection (Logical)
0x09, 0x26,        //     Usage (0x26)
0x09, 0x27,        //     Usage (0x27)
0x09, 0x30,        //     Usage (0x30)
0x09, 0x31,        //     Usage (0x31)
0x09, 0x32,        //     Usage (0x32)
0x09, 0x33,        //     Usage (0x33)
0x09, 0x34,        //     Usage (0x34)
0x09, 0x40,        //     Usage (0x40)
0x09, 0x41,        //     Usage (0x41)
0x09, 0x42,        //     Usage (0x42)
0x09, 0x43,        //     Usage (0x43)
0x09, 0x28,        //     Usage (0x28)
0x25, 0x0C,        //     Logical Maximum (12)
0x15, 0x01,        //     Logical Minimum (1)
0x35, 0x01,        //     Physical Minimum (1)
0x45, 0x0C,        //     Physical Maximum (12)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0x91, 0x00,        //     Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x09, 0x50,        //   Usage (0x50)
0x09, 0x54,        //   Usage (0x54)
0x09, 0x51,        //   Usage (0x51)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0xFF, 0x7F,  //   Physical Maximum (32767)
0x66, 0x03, 0x10,  //   Unit (System: English Linear, Time: Seconds)
0x55, 0xFD,        //   Unit Exponent
0x75, 0x10,        //   Report Size (16)
0x95, 0x03,        //   Report Count (3)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x55, 0x00,        //   Unit Exponent (0)
0x66, 0x00, 0x00,  //   Unit (None)
0x09, 0x52,        //   Usage (0x52)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x53,        //   Usage (0x53)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x08,        //   Logical Maximum (8)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x08,        //   Physical Maximum (8)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x55,        //   Usage (0x55)
0xA1, 0x02,        //   Collection (Logical)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x09, 0x30,        //     Usage (X)
0x09, 0x31,        //     Usage (Y)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x95, 0x02,        //     Report Count (2)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x05, 0x0F,        //   Usage Page (PID Page)
0x09, 0x56,        //   Usage (0x56)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x95, 0x05,        //   Report Count (5)
0x91, 0x03,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x57,        //   Usage (0x57)
0xA1, 0x02,        //   Collection (Logical)
0x0B, 0x01, 0x00, 0x0A, 0x00,  //     Usage (0x0A0001)
0x0B, 0x02, 0x00, 0x0A, 0x00,  //     Usage (0x0A0002)
0x66, 0x14, 0x00,  //     Unit (System: English Rotation, Length: Centimeter)
0x55, 0xFE,        //     Unit Exponent
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xB4, 0x00,  //     Logical Maximum (180)
0x35, 0x00,        //     Physical Minimum (0)
0x47, 0xA0, 0x8C, 0x00, 0x00,  //     Physical Maximum (35999)
0x66, 0x00, 0x00,  //     Unit (None)
0x75, 0x08,        //     Report Size (8)
0x95, 0x02,        //     Report Count (2)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x55, 0x00,        //     Unit Exponent (0)
0x66, 0x00, 0x00,  //     Unit (None)
0xC0,              //   End Collection
0x05, 0x0F,        //   Usage Page (PID Page)
0x66, 0x03, 0x10,  //   Unit (System: English Linear, Time: Seconds)
0x55, 0xFD,        //   Unit Exponent
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0xFF, 0x7F,  //   Physical Maximum (32767)
0x75, 0x10,        //   Report Size (16)
0x95, 0x01,        //   Report Count (1)
0x66, 0x00, 0x00,  //   Unit (None)
0x55, 0x00,        //   Unit Exponent (0)
0xC0,              // End Collection
0x05, 0x0F,        // Usage Page (PID Page)
0x09, 0x5A,        // Usage (0x5A)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x02,        //   Report ID (2)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x5B,        //   Usage (0x5B)
0x09, 0x5D,        //   Usage (0x5D)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x95, 0x02,        //   Report Count (2)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x5C,        //   Usage (0x5C)
0x09, 0x5E,        //   Usage (0x5E)
0x66, 0x03, 0x10,  //   Unit (System: English Linear, Time: Seconds)
0x55, 0xFD,        //   Unit Exponent
0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
0x46, 0xFF, 0x7F,  //   Physical Maximum (32767)
0x75, 0x10,        //   Report Size (16)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x45, 0x00,        //   Physical Maximum (0)
0x66, 0x00, 0x00,  //   Unit (None)
0x55, 0x00,        //   Unit Exponent (0)
0xC0,              // End Collection
0x09, 0x5F,        // Usage (0x5F)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x03,        //   Report ID (3)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x23,        //   Usage (0x23)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x35, 0x00,        //   Physical Minimum (0)
0x45, 0x01,        //   Physical Maximum (1)
0x75, 0x04,        //   Report Size (4)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x58,        //   Usage (0x58)
0xA1, 0x02,        //   Collection (Logical)
0x0B, 0x01, 0x00, 0x0A, 0x00,  //     Usage (0x0A0001)
0x0B, 0x02, 0x00, 0x0A, 0x00,  //     Usage (0x0A0002)
0x75, 0x02,        //     Report Size (2)
0x95, 0x02,        //     Report Count (2)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x15, 0x80,        //   Logical Minimum (-128)
0x25, 0x7F,        //   Logical Maximum (127)
0x36, 0xF0, 0xD8,  //   Physical Minimum (-10000)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x09, 0x60,        //   Usage (0x60)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x36, 0xF0, 0xD8,  //   Physical Minimum (-10000)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x09, 0x61,        //   Usage (0x61)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x09, 0x63,        //   Usage (0x63)
0x09, 0x64,        //   Usage (0x64)
0x75, 0x08,        //   Report Size (8)
0x95, 0x02,        //   Report Count (2)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x95, 0x01,        //   Report Count (1)
0xC0,              // End Collection
0x09, 0x6E,        // Usage (0x6E)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x04,        //   Report ID (4)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x70,        //   Usage (0x70)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x6F,        //   Usage (0x6F)
0x15, 0x80,        //   Logical Minimum (-128)
0x25, 0x7F,        //   Logical Maximum (127)
0x36, 0xF0, 0xD8,  //   Physical Minimum (-10000)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x71,        //   Usage (0x71)
0x66, 0x14, 0x00,  //   Unit (System: English Rotation, Length: Centimeter)
0x55, 0xFE,        //   Unit Exponent
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x35, 0x00,        //   Physical Minimum (0)
0x47, 0xA0, 0x8C, 0x00, 0x00,  //   Physical Maximum (35999)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x72,        //   Usage (0x72)
0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
0x46, 0xFF, 0x7F,  //   Physical Maximum (32767)
0x66, 0x03, 0x10,  //   Unit (System: English Linear, Time: Seconds)
0x55, 0xFD,        //   Unit Exponent
0x75, 0x10,        //   Report Size (16)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x66, 0x00, 0x00,  //   Unit (None)
0x55, 0x00,        //   Unit Exponent (0)
0xC0,              // End Collection
0x09, 0x73,        // Usage (0x73)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x05,        //   Report ID (5)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x70,        //   Usage (0x70)
0x16, 0x01, 0xFF,  //   Logical Minimum (-255)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x36, 0xF0, 0xD8,  //   Physical Minimum (-10000)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x75, 0x10,        //   Report Size (16)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x09, 0x74,        // Usage (0x74)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x06,        //   Report ID (6)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x75,        //   Usage (0x75)
0x09, 0x76,        //   Usage (0x76)
0x15, 0x80,        //   Logical Minimum (-128)
0x25, 0x7F,        //   Logical Maximum (127)
0x36, 0xF0, 0xD8,  //   Physical Minimum (-10000)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x75, 0x08,        //   Report Size (8)
0x95, 0x02,        //   Report Count (2)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x09, 0x68,        // Usage (0x68)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x07,        //   Report ID (7)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x6C,        //   Usage (0x6C)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0x10, 0x27,  //   Logical Maximum (10000)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x75, 0x10,        //   Report Size (16)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x69,        //   Usage (0x69)
0x15, 0x81,        //   Logical Minimum (-127)
0x25, 0x7F,        //   Logical Maximum (127)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0xFF, 0x00,  //   Physical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x0C,        //   Report Count (12)
0x92, 0x02, 0x01,  //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile,Buffered Bytes)
0xC0,              // End Collection
0x09, 0x66,        // Usage (0x66)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x08,        //   Report ID (8)
0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
0x09, 0x30,        //   Usage (X)
0x09, 0x31,        //   Usage (Y)
0x15, 0x81,        //   Logical Minimum (-127)
0x25, 0x7F,        //   Logical Maximum (127)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0xFF, 0x00,  //   Physical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x02,        //   Report Count (2)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x05, 0x0F,        // Usage Page (PID Page)
0x09, 0x77,        // Usage (0x77)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x0A,        //   Report ID (10)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x78,        //   Usage (0x78)
0xA1, 0x02,        //   Collection (Logical)
0x09, 0x79,        //     Usage (0x79)
0x09, 0x7A,        //     Usage (0x7A)
0x09, 0x7B,        //     Usage (0x7B)
0x15, 0x01,        //     Logical Minimum (1)
0x25, 0x03,        //     Logical Maximum (3)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0x91, 0x00,        //     Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x09, 0x7C,        //   Usage (0x7C)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0xFF, 0x00,  //   Physical Maximum (255)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x09, 0x90,        // Usage (0x90)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x0B,        //   Report ID (11)
0x09, 0x22,        //   Usage (0x22)
0x25, 0x28,        //   Logical Maximum (40)
0x15, 0x01,        //   Logical Minimum (1)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x09, 0x96,        // Usage (0x96)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x0C,        //   Report ID (12)
0x09, 0x97,        //   Usage (0x97)
0x09, 0x98,        //   Usage (0x98)
0x09, 0x99,        //   Usage (0x99)
0x09, 0x9A,        //   Usage (0x9A)
0x09, 0x9B,        //   Usage (0x9B)
0x09, 0x9C,        //   Usage (0x9C)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x06,        //   Logical Maximum (6)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x09, 0x7D,        // Usage (0x7D)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x0D,        //   Report ID (13)
0x09, 0x7E,        //   Usage (0x7E)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0x10, 0x27,  //   Physical Maximum (10000)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x09, 0x6B,        // Usage (0x6B)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x0E,        //   Report ID (14)
0x09, 0x22,        //   Usage (0x22)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x28,        //   Logical Maximum (40)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x6D,        //   Usage (0x6D)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0xFF, 0x00,  //   Physical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x51,        //   Usage (0x51)
0x66, 0x03, 0x10,  //   Unit (System: English Linear, Time: Seconds)
0x55, 0xFD,        //   Unit Exponent
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0xFF, 0x7F,  //   Physical Maximum (32767)
0x75, 0x10,        //   Report Size (16)
0x95, 0x01,        //   Report Count (1)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x55, 0x00,        //   Unit Exponent (0)
0x66, 0x00, 0x00,  //   Unit (None)
0xC0,              // End Collection
0x09, 0xAB,        // Usage (0xAB)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x05,        //   Report ID (5)
0x09, 0x25,        //   Usage (0x25)
0xA1, 0x02,        //   Collection (Logical)
0x09, 0x26,        //     Usage (0x26)
0x09, 0x27,        //     Usage (0x27)
0x09, 0x30,        //     Usage (0x30)
0x09, 0x31,        //     Usage (0x31)
0x09, 0x32,        //     Usage (0x32)
0x09, 0x33,        //     Usage (0x33)
0x09, 0x34,        //     Usage (0x34)
0x09, 0x40,        //     Usage (0x40)
0x09, 0x41,        //     Usage (0x41)
0x09, 0x42,        //     Usage (0x42)
0x09, 0x43,        //     Usage (0x43)
0x09, 0x28,        //     Usage (0x28)
0x25, 0x0C,        //     Logical Maximum (12)
0x15, 0x01,        //     Logical Minimum (1)
0x35, 0x01,        //     Physical Minimum (1)
0x45, 0x0C,        //     Physical Maximum (12)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0xB1, 0x00,        //     Feature (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
0x09, 0x3B,        //   Usage (Byte Count)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x01,  //   Logical Maximum (511)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0xFF, 0x01,  //   Physical Maximum (511)
0x75, 0x0A,        //   Report Size (10)
0x95, 0x01,        //   Report Count (1)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x75, 0x06,        //   Report Size (6)
0xB1, 0x01,        //   Feature (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x05, 0x0F,        // Usage Page (PID Page)
0x09, 0x89,        // Usage (0x89)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x06,        //   Report ID (6)
0x09, 0x22,        //   Usage (0x22)
0x25, 0x28,        //   Logical Maximum (40)
0x15, 0x01,        //   Logical Minimum (1)
0x35, 0x01,        //   Physical Minimum (1)
0x45, 0x28,        //   Physical Maximum (40)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x8B,        //   Usage (0x8B)
0xA1, 0x02,        //   Collection (Logical)
0x09, 0x8C,        //     Usage (0x8C)
0x09, 0x8D,        //     Usage (0x8D)
0x09, 0x8E,        //     Usage (0x8E)
0x25, 0x03,        //     Logical Maximum (3)
0x15, 0x01,        //     Logical Minimum (1)
0x35, 0x01,        //     Physical Minimum (1)
0x45, 0x03,        //     Physical Maximum (3)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0xB1, 0x00,        //     Feature (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x09, 0xAC,        //   Usage (0xAC)
0x15, 0x00,        //   Logical Minimum (0)
0x27, 0xFF, 0xFF, 0x00, 0x00,  //   Logical Maximum (65534)
0x35, 0x00,        //   Physical Minimum (0)
0x47, 0xFF, 0xFF, 0x00, 0x00,  //   Physical Maximum (65534)
0x75, 0x10,        //   Report Size (16)
0x95, 0x01,        //   Report Count (1)
0xB1, 0x00,        //   Feature (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0x09, 0x7F,        // Usage (0x7F)
0xA1, 0x02,        // Collection (Logical)
0x85, 0x07,        //   Report ID (7)
0x09, 0x80,        //   Usage (0x80)
0x75, 0x10,        //   Report Size (16)
0x95, 0x01,        //   Report Count (1)
0x15, 0x00,        //   Logical Minimum (0)
0x35, 0x00,        //   Physical Minimum (0)
0x27, 0xFF, 0xFF, 0x00, 0x00,  //   Logical Maximum (65534)
0x47, 0xFF, 0xFF, 0x00, 0x00,  //   Physical Maximum (65534)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x83,        //   Usage (0x83)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x46, 0xFF, 0x00,  //   Physical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0xA9,        //   Usage (0xA9)
0x09, 0xAA,        //   Usage (0xAA)
0x75, 0x01,        //   Report Size (1)
0x95, 0x02,        //   Report Count (2)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x35, 0x00,        //   Physical Minimum (0)
0x45, 0x01,        //   Physical Maximum (1)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x75, 0x06,        //   Report Size (6)
0x95, 0x01,        //   Report Count (1)
0xB1, 0x03,        //   Feature (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
*/
	0xC0               // End Collection (Application)
}; // 137 bytes total



// STATE

static uint8_t hid_service_buffer[2500];
static uint8_t device_id_sdp_service_buffer[100];
static const char hid_device_name[] = "HID PiGun-1";
static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint16_t hid_cid;
static uint8_t hid_boot_device = 0;


static enum {
	APP_BOOTING,
	APP_NOT_CONNECTED,
	APP_CONNECTING,
	APP_CONNECTED
} app_state = APP_BOOTING;


int nServers = 0;
bd_addr_t servers[3];

void* pigun_autoconnect(void* nullargs);
static btstack_timer_source_t heartbeat;
static void heartbeat_handler(btstack_timer_source_t* ts);

static int connectorState = 0;
static btstack_timer_source_t connectorBLINK;
static void connectorBLINK_handler(btstack_timer_source_t* ts);


int compare_servers(bd_addr_t a, bd_addr_t b) {
	for (int i = 0; i < 6; i++) {
		if (a[i] != b[i]) return 0;
	}
	return 1;
}

// HID Report sending
static void send_report() {

	// this is the report to send
	// I do now know that the first byte is there for?!
	uint8_t hid_report[] = { 0xa1, 0, 0, 0, 0, 0 };

	hid_report[1] = (global_pigun_report.x) & 0xff;
	hid_report[2] = (global_pigun_report.x >> 8) & 0xff;
	hid_report[3] = (global_pigun_report.y) & 0xff;
	hid_report[4] = (global_pigun_report.y >> 8) & 0xff;
	hid_report[5] = global_pigun_report.buttons;

	//printf("sending x=%i (%i %i) y=%i (%i %i) \n", global_pigun_report.x, hid_report[1], hid_report[2], global_pigun_report.y, hid_report[3], hid_report[4]);
	hid_device_send_interrupt_message(hid_cid, &hid_report[0], 6); // 6 = sizeof(hid_report)
}

// called when host sends an output report
void get_report(uint16_t hid_cid, hid_report_type_t report_type, int report_size, uint8_t *report){

	printf("Host HID output report:\n");
	printf("\tHID CID: %i\n", hid_cid);
	printf("\tReport Type: %i\n", report_type);
	printf("\tReport Size: %i\n", report_size);
	printf("\tReport Data: ");
	for (int i=0; i<report_size; i++) {
		printf("%#02X ",report[i]);
	}
	printf("\n");

}


static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t packet_size){
	UNUSED(channel);
	UNUSED(packet_size);
	uint8_t status;
	
	printf("got packet type: %i \n", packet_type);

    if (packet_type == L2CAP_DATA_PACKET){
      if(packet[0]==0xA2) { //output report data from host
		printf("\tOutput Report Data: ");
		for (int i=0; i<packet_size; i++) {
			printf("%#02X ",packet[i]);
		}
      }
    }
	if (packet_type != HCI_EVENT_PACKET) return;

	switch (hci_event_packet_get_type(packet)) {
	case BTSTACK_EVENT_STATE:
		if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
		app_state = APP_NOT_CONNECTED;
		break;

	case HCI_EVENT_USER_CONFIRMATION_REQUEST:
		// ssp: inform about user confirmation request
		log_info("SSP User Confirmation Request with numeric value '%06"PRIu32"'\n", hci_event_user_confirmation_request_get_numeric_value(packet));
		log_info("SSP User Confirmation Auto accept\n");
		break;

	// *** HID META EVENTS ***
	case HCI_EVENT_HID_META:
		switch (hci_event_hid_meta_get_subevent_code(packet)) {

		case HID_SUBEVENT_CONNECTION_OPENED:
			status = hid_subevent_connection_opened_get_status(packet);
			if (status != ERROR_CODE_SUCCESS) {
				// outgoing connection failed
				printf("PIGUN-HID: connection failed, status 0x%x\n", status);
				app_state = APP_NOT_CONNECTED;
				hid_cid = 0;

				// re-register timer
				btstack_run_loop_set_timer(&heartbeat, 1000);
				btstack_run_loop_add_timer(&heartbeat);
				return;
			}
			app_state = APP_CONNECTED;
			hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
			bd_addr_t host_addr;
			hid_subevent_connection_opened_get_bd_addr(packet, host_addr);

			
			// save the server address - if not already there
			// rewrite the past servers list, putting the current one on top
			// only write 3 of them
			int isThere = 0;
			for (int i = 0; i < nServers; i++) {
				if (compare_servers(host_addr, servers[i])) {
					isThere = 1;
					break;
				}
			}
			int ns = nServers;
			if (!isThere) ns++;
			if (ns > 3) ns = 3;
			int written = 1;
			bd_addr_t newlist[3];

			FILE* fout = fopen("servers.bin", "wb");
			fwrite(&ns, sizeof(int), 1, fout);
			fwrite(host_addr, sizeof(bd_addr_t), 1, fout); memcpy(newlist[0], host_addr, sizeof(bd_addr_t));

			for (int i = 0; i < nServers; i++) {
				if (!compare_servers(host_addr, servers[i])) {
					fwrite(servers[i], sizeof(bd_addr_t), 1, fout); memcpy(newlist[written], servers[i], sizeof(bd_addr_t));
					written++;
					if (written == 3) break;
				}
			}
			fclose(fout);
			memcpy(servers[0], newlist[0], sizeof(bd_addr_t)*3);
			nServers = ns;
			
			// once connected
			// turn off the green LED to save power
			pigun_GPIO_output_set(PIN_OUT_AOK, 0);

			printf("PIGUN-HID: connected to %s, pigunning now...\n", bd_addr_to_str(host_addr));
			hid_device_request_can_send_now_event(hid_cid); // request a sendnow
			break;
		case HID_SUBEVENT_CONNECTION_CLOSED:
			printf("PIGUN-HID: disconnected\n");
			app_state = APP_NOT_CONNECTED;
			hid_cid = 0;

			// start blinking of the green LED again
			btstack_run_loop_set_timer(&connectorBLINK, 800);
			btstack_run_loop_add_timer(&connectorBLINK);

			break;
		case HID_SUBEVENT_CAN_SEND_NOW:
			// when the stack raises can_send_now event, we send a report... because we can!
			send_report(); // uses the global variable with gun data

			// and then we request another can_send_now because we are greedy! need to send moooar!
			hid_device_request_can_send_now_event(hid_cid);
			break;
		default:
			break;
		}
		break;


	default: //any other type of event
		break;
	}
}



/* @section Main Application Setup
 *
 * @text Listing MainConfiguration shows main application code. 
 * To run a HID Device service you need to initialize the SDP, and to create and register HID Device record with it. 
 * At the end the Bluetooth stack is started.
 */

/* LISTING_START(MainConfiguration): Setup HID Device */

int btstack_main(int argc, const char * argv[]);
int btstack_main(int argc, const char * argv[]){
	(void)argc;
	(void)argv;

	// allow to get found by inquiry
	gap_discoverable_control(1);
	// use Limited Discoverable Mode; Peripheral; Keyboard as CoD
	gap_set_class_of_device(0x2504);
	// set local name to be identified - zeroes will be replaced by actual BD ADDR
	gap_set_local_name("PiGun 1F"); // ("PiGun 00:00:00:00:00:00");
	// allow for role switch in general and sniff mode
	gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
	// allow for role switch on outgoing connections - this allow HID Host to become master when we re-connect to it
	gap_set_allow_role_switch(true);

	// L2CAP
	l2cap_init();

	// SDP Server
	sdp_init();
	memset(hid_service_buffer, 0, sizeof(hid_service_buffer));

	uint8_t hid_virtual_cable = 0;
	uint8_t hid_remote_wake = 0;
	uint8_t hid_reconnect_initiate = 1;
	uint8_t hid_normally_connectable = 1;

	hid_sdp_record_t hid_params = {
		// hid sevice subclass 0x2504 joystick, hid counntry code 33 US
		0x2504, 33, // should be the joystick code
		hid_virtual_cable, hid_remote_wake, 
		hid_reconnect_initiate, hid_normally_connectable,
		hid_boot_device, 
		0xFFFF, 0xFFFF, 3200,
		hid_descriptor_joystick_mode,
		sizeof(hid_descriptor_joystick_mode), 
		hid_device_name
	};
	
	hid_create_sdp_record(hid_service_buffer, 0x10001, &hid_params);

	printf("PIGUN-HID: HID service record size: %u\n", de_get_len(hid_service_buffer));
	sdp_register_service(hid_service_buffer);

	// See https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers if you don't have a USB Vendor ID and need a Bluetooth Vendor ID
	// device info: BlueKitchen GmbH, product 1, version 1
   	device_id_create_sdp_record(device_id_sdp_service_buffer, 0x10003, 0x0001, 0x048F, 2, 1);
	printf("Device ID SDP service record size: %u\n", de_get_len((uint8_t*)device_id_sdp_service_buffer));
	sdp_register_service(device_id_sdp_service_buffer);

	// HID Device
	hid_device_init(hid_boot_device, sizeof(hid_descriptor_joystick_mode), hid_descriptor_joystick_mode);
	
	// register for HCI events
	hci_event_callback_registration.callback = &packet_handler;
	hci_add_event_handler(&hci_event_callback_registration);

	// register for HID events
	hid_device_register_packet_handler(&packet_handler);

	// sign up for host output reports
	hid_device_register_set_report_callback(&get_report);


	// turn on!
	hci_power_control(HCI_POWER_ON);

	
	// read the previous server addresses
	nServers = 0;
	FILE* fin = fopen("servers.bin", "rb");
	if (fin == NULL) {
		fin = fopen("servers.bin", "wb");
		fwrite(&nServers, sizeof(int), 1, fin);
		fclose(fin);
		fin = fopen("servers.bin", "rb");
	}
	fread(&nServers, sizeof(int), 1, fin);
	
	printf("PIGUN-HID: previous hosts: %i\n", nServers);
	for (int i = 0; i < nServers; i++) {
		fread(servers[i], sizeof(bd_addr_t), 1, fin);
		printf("\thost[%i]: %s\n", i, bd_addr_to_str(servers[i]));
	}
	fclose(fin);
	
	
	// start blinking of the green LED
	connectorBLINK.process = &connectorBLINK_handler;
	btstack_run_loop_set_timer(&connectorBLINK, 800);
	btstack_run_loop_add_timer(&connectorBLINK);

	// set one-shot timer for autoreconnect
	heartbeat.process = &heartbeat_handler;
	if (nServers != 0) {
		btstack_run_loop_set_timer(&heartbeat, 5000);
		btstack_run_loop_add_timer(&heartbeat);
	}

	return 0;
}


// This is called when the heartbeat times out.
// attempt to connect to a known host if app is not connected already
static void heartbeat_handler(btstack_timer_source_t* ts) {
	UNUSED(ts);

	// increment counter
	static int snum = 0;

	if (app_state == APP_NOT_CONNECTED && nServers != 0) {

		// try connecting to a server
		printf("PIGUN-HID: trying to connect to %s...\n", bd_addr_to_str(servers[snum]));
		hid_device_connect(servers[snum], &hid_cid);

		snum++; if (snum == nServers)snum = 0;
	}
}

static void connectorBLINK_handler(btstack_timer_source_t* ts) {
	UNUSED(ts);

	// change state and restart the blink timer if not connected
	if (app_state != APP_CONNECTED) {

		// switch state
		connectorState = (connectorState == 0) ? 1 : 0;

		pigun_GPIO_output_set(PIN_OUT_AOK, connectorState);

		btstack_run_loop_set_timer(&connectorBLINK, 800);
		btstack_run_loop_add_timer(&connectorBLINK);
	}
}

