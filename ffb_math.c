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
#include "ffb_math.h"

//implement cos and sin functions
float FfbCos(float angle)
{
	//return (int32_t)(arm_cos_f32(angle);
	return 0;
}

float FfbSin(float angle)
{
	//return (int32_t)(arm_sin_f32(angle);
  return 0;
}

int32_t ApplyGain(uint8_t value, uint8_t gain)
{
	int32_t value_32 = (int16_t)value;
	return ((value_32 * gain) / 255);
}

int32_t ApplyEnvelope(TEffectState effect, int32_t value)
{
	int32_t magnitude = ApplyGain(effect.magnitude, effect.gain);
	int32_t attackLevel = ApplyGain(effect.attackLevel, effect.gain);
	int32_t fadeLevel = ApplyGain(effect.fadeLevel, effect.gain);
	int32_t newValue = magnitude;
	int32_t attackTime = effect.attackTime;
	int32_t fadeTime = effect.fadeTime;
	int32_t elapsedTime = effect.elapsedTime;
	int32_t duration = effect.duration;

	if (elapsedTime < attackTime)
	{
		newValue = (magnitude - attackLevel) * elapsedTime;
		newValue /= attackTime;
		newValue += attackLevel;
	}
	if (elapsedTime > (duration - fadeTime))
	{
		newValue = (magnitude - fadeLevel) * (duration - elapsedTime);
		newValue /= fadeTime;
		newValue += fadeLevel;
	}
	
	newValue *= value;
	newValue /= 255;
	return newValue;
}

void ApplyDirection(TEffectState effect, int32_t force, int32_t* axes)
{
	float directionX = effect.directionX;
	float directionY = effect.directionY;
	if (effect.enableAxis == DIRECTION_ENABLE)
	{
		float angle = (directionX * 2) * DEG_TO_RAD;
		float fForce = force;
		axes[0] += (int32_t)(FfbCos(angle) * fForce);
		axes[1] += (int32_t)(FfbSin(angle) * fForce);
	}
	else
	{
		if (effect.enableAxis & X_AXIS_ENABLE)
		{
			float angle = (directionX * 2) * DEG_TO_RAD;
	  	float fForce = force;
			axes[0] += (int32_t)(FfbCos(angle) * fForce);
		}
		
		if (effect.enableAxis & Y_AXIS_ENABLE)
		{
			float angle = (directionY * 2) * DEG_TO_RAD;
	  	float fForce = force;
			axes[1] += (int32_t)(FfbSin(angle) * fForce);
		}
	}
}

void CalcCondition(TEffectState effect, int32_t * outValue, int32_t* inValue)
{
	float value[2] = {inValue[0],inValue[1]};
	if (effect.axesIdx > 1)
	{
		for (int32_t i = 0; i < effect.axesIdx; i++)
		{
			float deadBand = effect.deadBand[i];
      float cpOffset = effect.cpOffset[i];
			float negativeSaturation = -effect.negativeSaturation[i];
			float positiveSaturation = effect.positiveSaturation[i];
			float negativeCoefficient = effect.negativeCoefficient[i];
			float positiveCoefficient = effect.positiveCoefficient[i];
			
			if (value[i] < (cpOffset - deadBand))
			{
				float tempForce = (value[i] - (cpOffset - deadBand)) * negativeCoefficient;
				outValue[i] =(int32_t) (tempForce < negativeSaturation ? negativeSaturation : tempForce);
			}
			else if (value[i] > (cpOffset + deadBand))
			{
				float tempForce = (value[i]-(cpOffset + deadBand)) * positiveCoefficient;
				outValue[i] = (int32_t)(tempForce > positiveSaturation ? positiveSaturation : tempForce);
			} 
			else outValue[i]=0;
		}
	}
	if (effect.axesIdx == 1)
	{
		float tempForce = 0;
		float deadBand = effect.deadBand[0];
    float cpOffset = effect.cpOffset[0];
		float negativeSaturation = -effect.negativeSaturation[0];
		float positiveSaturation = effect.positiveSaturation[0];
		float negativeCoefficient = effect.negativeCoefficient[0];
		float positiveCoefficient = effect.positiveCoefficient[0];

		for (int32_t i = 0; i < effect.axesIdx; i++)
		{
			if (value[i] < (cpOffset - deadBand))
			{																	  
				tempForce = ((cpOffset - deadBand) - value[i]) * negativeCoefficient;																	  
				outValue[i] = (int32_t)(tempForce < negativeSaturation ? negativeSaturation : tempForce);
			}
			else if (value[i] > (cpOffset + deadBand))
			{
				tempForce = (value[i]-(cpOffset + deadBand)) * positiveCoefficient;
				outValue[i] = (int32_t)(tempForce > positiveSaturation ? positiveSaturation : tempForce);
			}
			else outValue[i] = 0;
		}
	}

}

