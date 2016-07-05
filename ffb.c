/*
Force Feedback Joystick
Joystick model specific code for handling force feedback data.
Copyright 2012  Tero Loimuneva (tloimu [at] gmail [dot] com)
Copyright 2013  Saku Kekkonen
Copyright 2016  Jaka Simonic    (telesimke [at] gmail [dot] com)
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

#include "ffb.h"
#include "ffb_math.h"

// Effect management
volatile uint8_t nextEID = 1;   // FFP effect indexes starts from 1
volatile TEffectState gEffectStates[MAX_EFFECTS + 1];
uint8_t GetNextFreeEffect(void);
void StartEffect(uint8_t id);
void StopEffect(uint8_t id);
void StopAllEffects(void);
void FreeEffect(uint8_t id);
void FreeAllEffects(void);
volatile uint8_t devicePaused=0;

//variables for storing previous values 
volatile int32_t inertiaT[2] = {0,0};
volatile int16_t oldSpeed[2] = {0,0};
volatile int16_t oldAxisPosition[2] = {0,0};

//ffb state structures
volatile USB_FFBReport_PIDStatus_Input_Data_t pidState = {2,30,0}; 
volatile USB_FFBReport_PIDBlockLoad_Feature_Data_t pidBlockLoad;
volatile USB_FFBReport_PIDPool_Feature_Data_t pidPoolReport;
volatile USB_FFBReport_DeviceGain_Output_Data_t deviceGain;

uint8_t GetNextFreeEffect(void)
{
	if (nextEID == MAX_EFFECTS)
	return 0;

	uint8_t id = nextEID++;

	// Find the next free effect ID for next time
	//nextEID=0;
	while (gEffectStates[nextEID].state != 0)
	{
		if (nextEID >= MAX_EFFECTS)
		break;  // the last spot was taken
		nextEID++;
	}

	gEffectStates[id].state = MEFFECTSTATE_ALLOCATED;

	return id;
}

void StopAllEffects(void)
{
	for (uint8_t id = 0; id <= MAX_EFFECTS; id++)
	StopEffect(id);
}

void StartEffect(uint8_t id)
{
	if (id > MAX_EFFECTS)
	return;
	gEffectStates[id].state |= MEFFECTSTATE_PLAYING;
	gEffectStates[id].elapsedTime = 0;
}

void StopEffect(uint8_t id)
{
	if (id > MAX_EFFECTS)
	return;
	gEffectStates[id].state &= ~MEFFECTSTATE_PLAYING;
	pidBlockLoad.ramPoolAvailable += SIZE_EFFECT;
	
}

void FreeEffect(uint8_t id)
{
	if (id > MAX_EFFECTS)
	   return;

	gEffectStates[id].state = 0;
	if (id < nextEID)
	   nextEID = id;

}

void FreeAllEffects(void)
{
	nextEID = 1;
	memset((void*)&gEffectStates, 0, sizeof(gEffectStates));
	pidBlockLoad.ramPoolAvailable = MEMORY_SIZE;
}

void FfbHandle_EffectOperation(USB_FFBReport_EffectOperation_Output_Data_t* data);
void FfbHandle_BlockFree(USB_FFBReport_BlockFree_Output_Data_t* data);
void FfbHandle_DeviceControl(USB_FFBReport_DeviceControl_Output_Data_t* data);
void FfbHandle_DeviceGain(USB_FFBReport_DeviceGain_Output_Data_t* data);
void FfbHandle_SetCustomForceData(USB_FFBReport_SetCustomForceData_Output_Data_t* data);
void FfbHandle_SetDownloadForceSample(USB_FFBReport_SetDownloadForceSample_Output_Data_t* data);
void FfbHandle_SetCustomForce(USB_FFBReport_SetCustomForce_Output_Data_t* data);
void FfbHandle_SetEffect(USB_FFBReport_SetEffect_Output_Data_t* data);
void SetEnvelope(USB_FFBReport_SetEnvelope_Output_Data_t* data, volatile TEffectState* effect);
void SetCondition(USB_FFBReport_SetCondition_Output_Data_t* data, volatile TEffectState* effect);
void SetPeriodic(USB_FFBReport_SetPeriodic_Output_Data_t* data, volatile TEffectState* effect);
void SetConstantForce(USB_FFBReport_SetConstantForce_Output_Data_t* data, volatile TEffectState* effect);
void SetRampForce(USB_FFBReport_SetRampForce_Output_Data_t* data, volatile TEffectState* effect);


// Handle incoming data from USB 
void FfbOnUsbData(uint8_t* data, uint16_t len)
{

	uint8_t effectId = data[1]; // effectBlockIndex is always the second byte.

	switch (data[0])    // reportID
	{
	case 1:
		FfbHandle_SetEffect((USB_FFBReport_SetEffect_Output_Data_t*)data);
		break;
	case 2:
		SetEnvelope((USB_FFBReport_SetEnvelope_Output_Data_t*)data, &gEffectStates[effectId]);
		break;
	case 3:
		SetCondition((USB_FFBReport_SetCondition_Output_Data_t*)data, &gEffectStates[effectId]);
		break;
	case 4:
		SetPeriodic((USB_FFBReport_SetPeriodic_Output_Data_t*)data, &gEffectStates[effectId]);
		break;
	case 5:
		SetConstantForce((USB_FFBReport_SetConstantForce_Output_Data_t*)data, &gEffectStates[effectId]);
		break;
	case 6:
		SetRampForce((USB_FFBReport_SetRampForce_Output_Data_t*)data, &gEffectStates[effectId]);
		break;
	case 7:
		FfbHandle_SetCustomForceData((USB_FFBReport_SetCustomForceData_Output_Data_t*)data);
		break;
	case 8:
		FfbHandle_SetDownloadForceSample((USB_FFBReport_SetDownloadForceSample_Output_Data_t*)data);
		break;
	case 9:
		break;
	case 10:
		FfbHandle_EffectOperation((USB_FFBReport_EffectOperation_Output_Data_t*)data);
		break;
	case 11:
		FfbHandle_BlockFree((USB_FFBReport_BlockFree_Output_Data_t*)data);
		break;
	case 12:
		FfbHandle_DeviceControl((USB_FFBReport_DeviceControl_Output_Data_t*)data);
		break;
	case 13:
		FfbHandle_DeviceGain((USB_FFBReport_DeviceGain_Output_Data_t*)data);
		break;
	case 14:
		FfbHandle_SetCustomForce((USB_FFBReport_SetCustomForce_Output_Data_t*)data);
		break;
	default:
		break;
	};

}

void SetRampForce(USB_FFBReport_SetRampForce_Output_Data_t* data, volatile TEffectState* effect)
{
	effect->start = data->start;
	effect->end = data->end;
}

void SetConstantForce(USB_FFBReport_SetConstantForce_Output_Data_t* data, volatile TEffectState* effect)
{
	effect->magnitude = data->magnitude;
}

void SetPeriodic(USB_FFBReport_SetPeriodic_Output_Data_t* data, volatile TEffectState* effect)
{
	effect->magnitude = data->magnitude;
	effect->offset = data->offset;
	effect->phase = data->phase;  
	effect->period = data->period;
}

void SetCondition(USB_FFBReport_SetCondition_Output_Data_t* data, volatile TEffectState* effect)
{
	effect->cpOffset[data->parameterBlockOffset] = data->cpOffset;   
	effect->positiveCoefficient[data->parameterBlockOffset] = data->positiveCoefficient;
	effect->negativeCoefficient[data->parameterBlockOffset] = data->negativeCoefficient;
	effect->positiveSaturation[data->parameterBlockOffset] = data->positiveSaturation;
	effect->negativeSaturation[data->parameterBlockOffset] = data->negativeSaturation;
	effect->deadBand[data->parameterBlockOffset] = data->deadBand;    
	effect->axesIdx = (data->parameterBlockOffset + 1 > effect->axesIdx) ? (data->parameterBlockOffset + 1) : (effect->axesIdx);
}

void SetEnvelope(USB_FFBReport_SetEnvelope_Output_Data_t* data, volatile TEffectState* effect)
{
	effect->attackLevel = data->attackLevel;
	effect->fadeLevel = data->fadeLevel;
	effect->attackTime = data->attackTime;
	effect->fadeTime = data->fadeTime;
}

void FfbOnCreateNewEffect(USB_FFBReport_CreateNewEffect_Feature_Data_t* inData)
{
	pidBlockLoad.reportId = 6;
	pidBlockLoad.effectBlockIndex = GetNextFreeEffect();

	if (pidBlockLoad.effectBlockIndex == 0)
	{
		pidBlockLoad.loadStatus = 2;    // 1=Success,2=Full,3=Error
	}
	else
	{
		pidBlockLoad.loadStatus = 1;    // 1=Success,2=Full,3=Error

		volatile TEffectState* effect = &gEffectStates[pidBlockLoad.effectBlockIndex];

		memset((void*)effect, 0, sizeof(TEffectState));
		effect->state = MEFFECTSTATE_ALLOCATED;
		pidBlockLoad.ramPoolAvailable -= SIZE_EFFECT;
	}
}

void FfbHandle_SetEffect(USB_FFBReport_SetEffect_Output_Data_t* data)
{
	volatile TEffectState* effect = &gEffectStates[data->effectBlockIndex];

	effect->duration = data->duration;  
	effect->directionX = data->directionX;
	effect->directionY = data->directionY;
	effect->effectType = data->effectType;
	effect->gain = data->gain;
	effect->enableAxis = data->enableAxis;
//	effect->triggerRepeatInterval;
//	effect->samplePeriod;   // 0..32767 ms
//	effect->triggerButton;    
}

uint8_t* FfbOnPIDPool()
{
	FreeAllEffects();

	pidPoolReport.reportId = 7;
	pidPoolReport.ramPoolSize = MEMORY_SIZE;
	pidPoolReport.maxSimultaneousEffects = MAX_EFFECTS;    
	pidPoolReport.memoryManagement = 3;
	return (uint8_t*)&pidPoolReport;
}

uint8_t* FfbOnPIDBlockLoad()
{
	return (uint8_t*)&pidBlockLoad;
}

uint8_t* FfbOnPIDStatus()
{
	return (uint8_t*)&pidState;
}

void FfbHandle_SetCustomForceData(USB_FFBReport_SetCustomForceData_Output_Data_t* data)
{
}

void FfbHandle_SetDownloadForceSample(USB_FFBReport_SetDownloadForceSample_Output_Data_t* data)
{
}

void FfbHandle_EffectOperation(USB_FFBReport_EffectOperation_Output_Data_t* data)
{
	if (data->operation == 1)
	{   // Start
		if(data->loopCount > 0) gEffectStates[data->effectBlockIndex].duration *= data->loopCount;
		if(data->loopCount==0xFF) gEffectStates[data->effectBlockIndex].duration = USB_DURATION_INFINITE;
		StartEffect(data->effectBlockIndex);
	}
	else if (data->operation == 2)
	{   // StartSolo

		// Stop all first
		StopAllEffects();

		// Then start the given effect
		StartEffect(data->effectBlockIndex);

	}
	else if (data->operation == 3)
	{   // Stop

		StopEffect(data->effectBlockIndex);
	}
	else
	{
	}
}

void FfbHandle_BlockFree(USB_FFBReport_BlockFree_Output_Data_t* data)
{
	uint8_t eid = data->effectBlockIndex;

	if (eid == 0xFF)
	{   // all effects
		FreeAllEffects();
	}
	else
	{
		FreeEffect(eid);
	}
}

void FfbHandle_DeviceControl(USB_FFBReport_DeviceControl_Output_Data_t* data)
{

	uint8_t control = data->control; 

	if (control == 0x01)
	{ // 1=Enable Actuators
		pidState.status |= 2;
	}
	else if (control == 0x02)
	{// 2=Disable Actuators
		pidState.status &= ~(0x02);
	}
	else if (control == 0x03)
	{ // 3=Stop All Effects
		StopAllEffects();
	}
	else if (control == 0x04)
	{ //  4=Reset
		FreeAllEffects();
	}
	else if (control == 0x05)
	{ // 5=Pause
		devicePaused = 1;
	}
	else if (control == 0x06)
	{ // 6=Continue
		devicePaused = 0;
	}
	else if (control & (0xFF - 0x3F))
	{
	}
}

void FfbHandle_DeviceGain(USB_FFBReport_DeviceGain_Output_Data_t* data)
{
	deviceGain.gain = data->gain;
}


void FfbHandle_SetCustomForce(USB_FFBReport_SetCustomForce_Output_Data_t* data)
{
}


void FfbGetFeedbackValue(int16_t* axisPosition, int16_t* out)
{
	int32_t x_y_force[2] = {0,0};
	
	for (uint8_t id = 0; id <= MAX_EFFECTS; id++)
	{
		TEffectState effect = gEffectStates[id];

		if ((effect.state & MEFFECTSTATE_PLAYING) && ((effect.elapsedTime <= effect.duration)||(effect.duration == USB_DURATION_INFINITE)) && !devicePaused)
		{
			switch (effect.effectType)
			{
			case USB_EFFECT_CONSTANT:
				{
					int32_t tforce = ApplyEnvelope(effect, (int32_t)effect.magnitude);
					ApplyDirection(effect, tforce, x_y_force);
				}
				break;
			case USB_EFFECT_RAMP:
				{
					int32_t end = effect.end * 2;
					int32_t start = effect.start * 2;
					uint32_t elapsedTime = effect.elapsedTime;
					int32_t duration = effect.duration;
					
					int32_t temp = (end - start) * elapsedTime;
					temp /= duration;
					temp += start;
					ApplyDirection(effect, temp, x_y_force);
				}
				break;
			case USB_EFFECT_SQUARE:
				{
					int32_t offset = effect.offset * 2;
					uint32_t magnitude = effect.magnitude;
					uint32_t elapsedTime = effect.elapsedTime;
					uint32_t phase = effect.phase;
					uint32_t period = effect.period;

					int32_t max = offset + magnitude;
					int32_t min = offset - magnitude;
					uint32_t phasetime = (phase * period) / 255;
					uint32_t time = elapsedTime + phasetime;
					uint32_t reminder = time % period;
					int32_t tempforce;
					if (reminder > (period / 2)) tempforce = min;
					else tempforce = max;
					ApplyDirection(effect, ApplyEnvelope(effect, tempforce), x_y_force);
				}
				break;
			case USB_EFFECT_SINE:
				{
					float offset = effect.offset * 2;
					float magnitude = effect.magnitude;
					float phase = effect.phase;
					float time = effect.elapsedTime;
					float period = effect.period;
					
					float angle = ((time / period) + (phase / 255) * period) * 2 * PI;
					float sine = FfbSin(angle);
					float tempforce = sine * magnitude;
					tempForce += offset;
					ApplyDirection(effect, ApplyEnvelope(effect, (int32_t)tempforce), x_y_force);
				}
				break;
			case USB_EFFECT_TRIANGLE:
				{
					float offset = effect.offset*2;
					float magnitude = effect.magnitude;
					float elapsedTime=effect.elapsedTime;
					uint32_t phase=effect.phase;
					uint32_t period = effect.period;
					float periodF = effect.period;
					
					float max = offset + magnitude;
					float min = offset - magnitude;
					uint32_t phasetime = (phase * period) / 255;
					uint32_t time = elapsedTime + phasetime;
					float reminder = time % period;
					float slope = ((max - min)*2) / periodF;
					float tempforce = 0;
					if (reminder > (periodF / 2)) tempforce = slope * (periodF - reminder);
					else tempforce = slope * reminder;
					tempforce += min;
					ApplyDirection(effect, ApplyEnvelope(effect, tempforce), x_y_force);
				}

				break;
			case USB_EFFECT_SAWTOOTHDOWN:
				{
					float offset = effect.offset*2;
					float magnitude = effect.magnitude;
					float elapsedTime=effect.elapsedTime;
					float phase=effect.phase;
					uint32_t period = effect.period;
					float periodF = effect.period;
					
					float max = offset + magnitude;
					float min = offset - magnitude;
					int32_t phasetime = (phase * period) / 255;
					uint32_t time = elapsedTime + phasetime;
					float reminder = time % period;
					float slope = (max - min) / periodF;
					float tempforce = 0;
					tempforce = slope * (period - reminder);
					tempforce += min;
					ApplyDirection(effect, ApplyEnvelope(effect, tempforce), x_y_force);
				}

				break;
			case USB_EFFECT_SAWTOOTHUP:
				{
					float offset = effect.offset*2;
					float magnitude = effect.magnitude;
					float elapsedTime=effect.elapsedTime;
					uint32_t phase=effect.phase;
					uint32_t period = effect.period;
					float periodF = effect.period;
					
					float max = offset + magnitude;
					float min = offset - magnitude;
					int32_t phasetime = (phase * period) / 255;
					uint32_t time = elapsedTime + phasetime;
					float reminder = time % period;
					float slope = (max - min) / periodF;
					float tempforce = 0;
					tempforce = slope * reminder;
					tempforce += min;
					ApplyDirection(effect, ApplyEnvelope(effect, tempforce), x_y_force);
				}

				break;
			case USB_EFFECT_SPRING:
				{
					int32_t axis[2] = {NORMALIZE_RANGE(axisPosition[0]),NORMALIZE_RANGE(axisPosition[1])};
					int32_t temp[2]={0,0};
					CalcCondition(effect,temp,axis);
					x_y_force[0] += -temp[0];
					x_y_force[1] += -temp[1];
				}
				break;
			case USB_EFFECT_DAMPER:
				{
					int32_t speed[2] = {NORMALIZE_RANGE(axisPosition[0]) - NORMALIZE_RANGE(oldAxisPosition[0]), NORMALIZE_RANGE(axisPosition[1]) - NORMALIZE_RANGE(oldAxisPosition[1])};
					int32_t temp[2]={0,0};
					CalcCondition(effect,temp,speed);
					x_y_force[0] += -temp[0];
					x_y_force[1] += -temp[1];				
				}
				break;
			case USB_EFFECT_INERTIA:
				{
					int32_t speed[2] = {NORMALIZE_RANGE(axisPosition[0]) - NORMALIZE_RANGE(oldAxisPosition[0]), NORMALIZE_RANGE(axisPosition[1]) - NORMALIZE_RANGE(oldAxisPosition[1])};
					int32_t acceleration[2] = {speed[0] - oldSpeed[0], speed[1] - oldSpeed[1]};
					int32_t temp[2] = {0, 0};
					memset((void*)&effect.positiveSaturation, INERTIA_FORCE, 4);
					memset((void*)&effect.deadBand, INERTIA_DEADBAND, 2);
					CalcCondition(effect,temp,speed);
					inertiaT[0] += temp[0];
					inertiaT[1] += temp[1];
					x_y_force[0] += inertiaT[0];
					x_y_force[1] += inertiaT[1];
					if (speed[0] == 0) inertiaT[0] = 0;
					if (speed[1] == 0) inertiaT[1] = 0; 
					                               
				}
				break;
			case USB_EFFECT_FRICTION:
				{
					int32_t speed[2] = {NORMALIZE_RANGE(axisPosition[0]) - NORMALIZE_RANGE(oldAxisPosition[0]), NORMALIZE_RANGE(axisPosition[1]) - NORMALIZE_RANGE(oldAxisPosition[1])};
					int32_t temp[2] = {0, 0};
					memset((void*)&effect.positiveSaturation, FRICTION_FORCE, 4);
					memset((void*)&effect.deadBand, FRICTION_DEADBAND, 2);
					CalcCondition(effect,temp,speed);
					x_y_force[0] += -temp[0];
					x_y_force[1] += -temp[1];				
				}
				break;
			case USB_EFFECT_CUSTOM:
				break;
			}
		gEffectStates[id].elapsedTime += WHEEL_SAMPLE_RATE_MS;
		}
	}
	
	uint32_t gain = deviceGain.gain;
	for(uint32_t i = 0;i < 2; i++)
	{
	  x_y_force[i] *= gain;
	  x_y_force[i] /= 255;
		out[i] = x_y_force[i] > 255 ? 255 : x_y_force[i];
		out[i] = x_y_force[i] < -255 ? -255 : x_y_force[i];
	}
	
	oldSpeed[0] = axisPosition[0] - oldAxisPosition[0];
	oldSpeed[1] = axisPosition[1] - oldAxisPosition[1];

}


