/*
Force Feedback Joystick Math
Joystick model specific code for calculating force feedback.
Copyright 2016  Jaka Simonic
Permission to use, copy, modify, distribute, and sell this
software and its documentation for any purpose is hereby granted
without fee, provided that the above copyright notice appear in
all copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of the author not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
The author disclaim all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall the author be liable for any
special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
*/

#ifndef _FFB_MATH_
#define _FFB_MATH_

#include "ffb.h"
//#include "arm_math.h"

#define WHEEL_SAMPLE_RATE_MS 	  10
#define WHEEL_RANGE   					0x03B7

#define PI                      (float)3.14159265359 //consider using library defined value of PI
#define DEG_TO_RAD     					((float)(PI/180))
#define NORMALIZE_RANGE(x) 		 ((int32_t)((x*255)/WHEEL_RANGE))	

//implement cos and sin functions
float FfbCos(float angle);
float FfbSin(float angle);

int32_t ApplyGain(uint8_t value, uint8_t gain);
int32_t ApplyEnvelope(TEffectState effect, int32_t value);
void ApplyDirection(TEffectState effect, int32_t force, int32_t* axes);
void CalcCondition(TEffectState effect, int32_t * outValue, int32_t* inValue);

#endif 

