/*
  Force Feedback Joystick
  Joystick model specific code for handling force feedback data.
  Copyright 2012  Tero Loimuneva (tloimu [at] gmail [dot] com)
  Copyright 2013  Saku Kekkonen
  Copyright 2016  Jaka Simonic    (telesimke [at] gmail [dot] com)
  Copyright 2019  Hoan Tran (tranvanhoan206 [at] gmail [dot] com)
  Copyright 2025  Jaka Simonic    (telesimke [at] gmail [dot] com)
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

#include "FfbReportHandler.h"
#include <string.h>
#include <math.h>

FfbReportHandler::FfbReportHandler(uint64_t (*pTime)(void)) : getTimeMilli{pTime}
{
  devicePaused = 0;
  pauseTime = 0;
  FreeAllEffects();
}

FfbReportHandler::~FfbReportHandler()
{
  FreeAllEffects();
}

TEffectState *FfbReportHandler::GetEffect(uint8_t id)
{
  if (id > 0 && id <= MAX_EFFECTS)
  {
    return &gEffectStates[id - 1];
  }
  return nullptr;
}

const TEffectState *FfbReportHandler::GetEffectStates()
{
  return (const TEffectState *)gEffectStates;
}

uint8_t FfbReportHandler::GetNextFreeEffect(void)
{
  for (int id = 0; id < MAX_EFFECTS; ++id)
  {
    if (gEffectStates[id].state == MEFFECTSTATE_FREE)
    {
      return id + 1;
    }
  }

  return 0;
}

void FfbReportHandler::StopAllEffects(void)
{
  for (uint8_t id = 0; id < MAX_EFFECTS; ++id)
    StopEffect(&gEffectStates[id]);
}

void FfbReportHandler::StartEffect(TEffectState *effectState)
{
  effectState->state = MEFFECTSTATE_PLAYING;
  if (effectState->block.triggerButton != 0xFF)
    effectState->startTime = 0;
  else
    effectState->startTime = getTimeMilli() + effectState->block.startDelay;
}

void FfbReportHandler::StopEffect(TEffectState *effectState)
{
  effectState->state &= ~MEFFECTSTATE_PLAYING;
}

void FfbReportHandler::FreeEffect(uint8_t id)
{
  volatile TEffectState *effectState = GetEffect(id);
  if (nullptr == effectState)
    return;
  effectState->state = MEFFECTSTATE_FREE;
  pidBlockLoad.ramPoolAvailable += SIZE_EFFECT;
}

void FfbReportHandler::FreeAllEffects(void)
{
  memset((void *)&gEffectStates, 0, sizeof(gEffectStates));
  pidBlockLoad.ramPoolAvailable = MEMORY_SIZE;
}

void FfbReportHandler::FfbHandle_EffectOperation(USB_FFBReport_EffectOperation_Output_Data_t *data)
{
  uint8_t effectBlockIndex = data->effectBlockIndex;
  uint8_t operation = data->operation;
  TEffectState *effectState = GetEffect(effectBlockIndex);
  switch (operation)
  {
  case 1:
    // Start
    if (data->loopCount > 0)
      effectState->block.duration *= data->loopCount;
    if (data->loopCount == 0xFF)
      effectState->block.duration = USB_DURATION_INFINITE;
    StartEffect(effectState);
    break;

  case 2:
    // StartSolo
    // Stop all first
    StopAllEffects();

    // Then start the given effect
    StartEffect(effectState);
    break;
  case 3:
    // Stop
    StopEffect(effectState);
  }
}

void FfbReportHandler::FfbHandle_BlockFree(USB_FFBReport_BlockFree_Output_Data_t *data)
{
  uint8_t eid = data->effectBlockIndex;

  if (eid == 0xFF)
  { // all effects
    FreeAllEffects();
  }
  else
  {
    FreeEffect(eid);
  }
}

void FfbReportHandler::FfbHandle_DeviceControl(USB_FFBReport_DeviceControl_Output_Data_t *data)
{

  uint8_t control = data->control;

  switch (control)
  {
  case 1:
    // 1=Enable Actuators
    pidState.status |= 2;
    break;
  case 2:
    // 2=Disable Actuators
    pidState.status &= ~(0x02);
    break;
  case 3:
    // 3=Stop All Effects
    StopAllEffects();
    break;
  case 4:
    //  4=Reset
    FreeAllEffects();
    break;
  case 5:
    // 5=Pause
    devicePaused = 1;
    pauseTime = getTimeMilli();
    break;
  case 6:
    // 6=Continue
    devicePaused = 0;

    if (pauseTime == 0)
      break;

    uint32_t pauseLength = getTimeMilli() - pauseTime;
    for (int id = 0; id < MAX_EFFECTS; ++id)
    {
      if (gEffectStates[id].state & MEFFECTSTATE_PLAYING)
      {
        if (pauseTime <= gEffectStates[id].startTime)
          continue;
        gEffectStates[id].startTime += pauseLength;
      }
    }
    pauseTime = 0;
    break;
  }
}
void FfbReportHandler::FfbHandle_DeviceGain(USB_FFBReport_DeviceGain_Output_Data_t *data)
{
  deviceGain = data->gain;
}

void FfbReportHandler::FfbHandle_SetCustomForce(USB_FFBReport_SetCustomForce_Output_Data_t *data)
{
}

void FfbReportHandler::FfbHandle_SetCustomForceData(USB_FFBReport_SetCustomForceData_Output_Data_t *data)
{
}

void FfbReportHandler::FfbHandle_SetDownloadForceSample(USB_FFBReport_SetDownloadForceSample_Output_Data_t *data)
{
}

void FfbReportHandler::FfbHandle_SetEffect(USB_FFBReport_SetEffect_Output_Data_t *data)
{
  TEffectState *effectState = GetEffect(data->effectBlockIndex);
  USB_FFBReport_SetEffect_Output_Data_t *block = &effectState->block;
  memcpy((void *)block, data, sizeof(USB_FFBReport_SetEffect_Output_Data_t));

  float normalizedDirectionX = data->directionX / USB_NORMALIZE_RAD;
  float normalizedDirectionY = data->directionY / USB_NORMALIZE_RAD;
  uint8_t enableAxis = data->enableAxis;
  if (enableAxis & DIRECTION_ENABLE)
  {
    effectState->directionUnitVec[0] = cos(normalizedDirectionX);
    effectState->directionUnitVec[1] = sin(normalizedDirectionX);
  }
  else
  {
    effectState->directionUnitVec[0] = cos(normalizedDirectionX);
    effectState->directionUnitVec[1] = sin(normalizedDirectionY);
  }
}

void FfbReportHandler::SetEnvelope(USB_FFBReport_SetEnvelope_Output_Data_t *data)
{
  TEffectState *effectState = GetEffect(data->effectBlockIndex);
  USB_FFBReport_SetEnvelope_Output_Data_t *periodic = &effectState->parameters[TYPE_SPECIFIC_BLOCK_OFFSET_2].envelope;
  memcpy((void *)periodic, data, sizeof(USB_FFBReport_SetEnvelope_Output_Data_t));
}

void FfbReportHandler::SetCondition(USB_FFBReport_SetCondition_Output_Data_t *data)
{
  uint8_t parameterBlockOffset = data->parameterBlockOffset & 0x0F;
  if (parameterBlockOffset > NUM_AXES - 1)
    return;

  TEffectState *effectState = GetEffect(data->effectBlockIndex);
  USB_FFBReport_SetCondition_Output_Data_t *condition = &effectState->parameters[parameterBlockOffset].condition;
  memcpy((void *)condition, data, sizeof(USB_FFBReport_SetCondition_Output_Data_t));
}

void FfbReportHandler::SetPeriodic(USB_FFBReport_SetPeriodic_Output_Data_t *data)
{
  TEffectState *effectState = GetEffect(data->effectBlockIndex);
  USB_FFBReport_SetPeriodic_Output_Data_t *periodic = &effectState->parameters[TYPE_SPECIFIC_BLOCK_OFFSET_1].periodic;
  memcpy((void *)periodic, data, sizeof(USB_FFBReport_SetPeriodic_Output_Data_t));
  effectState->envelopeParameter = true;
}

void FfbReportHandler::SetConstantForce(USB_FFBReport_SetConstantForce_Output_Data_t *data)
{
  TEffectState *effectState = GetEffect(data->effectBlockIndex);
  USB_FFBReport_SetConstantForce_Output_Data_t *constant = &effectState->parameters[TYPE_SPECIFIC_BLOCK_OFFSET_1].constant;
  memcpy((void *)constant, data, sizeof(USB_FFBReport_SetConstantForce_Output_Data_t));
}

void FfbReportHandler::SetRampForce(USB_FFBReport_SetRampForce_Output_Data_t *data)
{
  TEffectState *effectState = GetEffect(data->effectBlockIndex);
  USB_FFBReport_SetRampForce_Output_Data_t *ramp = &effectState->parameters[TYPE_SPECIFIC_BLOCK_OFFSET_1].ramp;
  memcpy((void *)ramp, data, sizeof(USB_FFBReport_SetRampForce_Output_Data_t));
}

void FfbReportHandler::FfbOnCreateNewEffect(USB_FFBReport_CreateNewEffect_Feature_Data_t *inData)
{
  pidBlockLoad.reportId = 6;
  pidBlockLoad.effectBlockIndex = GetNextFreeEffect();

  if (pidBlockLoad.effectBlockIndex == 0)
  {
    pidBlockLoad.loadStatus = 2; // 1=Success,2=Full,3=Error
  }
  else
  {
    pidBlockLoad.loadStatus = 1; // 1=Success,2=Full,3=Error

    volatile TEffectState *effectState = GetEffect(pidBlockLoad.effectBlockIndex);

    memset((void *)effectState, 0, sizeof(TEffectState));
    effectState->state = MEFFECTSTATE_ALLOCATED;

    pidBlockLoad.ramPoolAvailable -= SIZE_EFFECT;
  }
}

uint8_t *FfbReportHandler::FfbOnPIDPool()
{
  FreeAllEffects();

  pidPoolReport.reportId = 7;
  pidPoolReport.ramPoolSize = MEMORY_SIZE;
  pidPoolReport.maxSimultaneousEffects = MAX_EFFECTS;
  pidPoolReport.memoryManagement = 3;
  return (uint8_t *)&pidPoolReport;
}

uint8_t *FfbReportHandler::FfbOnPIDBlockLoad()
{
  return (uint8_t *)&pidBlockLoad;
}

uint8_t *FfbReportHandler::FfbOnPIDStatus()
{
  return (uint8_t *)&pidState;
}

void FfbReportHandler::FfbOnUsbData(uint8_t *data, uint16_t len)
{

  uint8_t effectId = data[1]; // effectBlockIndex is always the second byte.
  switch (data[0])            // reportID
  {
  case SET_EFFECT_REPORT:
    FfbHandle_SetEffect((USB_FFBReport_SetEffect_Output_Data_t *)data);
    break;
  case SET_ENVELOPE_REPORT:
    SetEnvelope((USB_FFBReport_SetEnvelope_Output_Data_t *)data);
    break;
  case SET_CONDITION_REPORT:
    SetCondition((USB_FFBReport_SetCondition_Output_Data_t *)data);
    break;
  case SET_PERIODIC_REPORT:
    SetPeriodic((USB_FFBReport_SetPeriodic_Output_Data_t *)data);
    break;
  case SET_CONSTANT_FORCE_REPORT:
    SetConstantForce((USB_FFBReport_SetConstantForce_Output_Data_t *)data);
    break;
  case SET_RAMP_FORCE_REPORT:
    SetRampForce((USB_FFBReport_SetRampForce_Output_Data_t *)data);
    break;
  case 7:
    FfbHandle_SetCustomForceData((USB_FFBReport_SetCustomForceData_Output_Data_t *)data);
    break;
  case 8:
    FfbHandle_SetDownloadForceSample((USB_FFBReport_SetDownloadForceSample_Output_Data_t *)data);
    break;
  case 9:
    break;
  case SET_EFFECT_OPERATION_REPORT:
    FfbHandle_EffectOperation((USB_FFBReport_EffectOperation_Output_Data_t *)data);
    break;
  case SET_BLOCK_FREE_REPORT:
    FfbHandle_BlockFree((USB_FFBReport_BlockFree_Output_Data_t *)data);
    break;
  case SET_DEVICE_FREE_REPORT:
    FfbHandle_DeviceControl((USB_FFBReport_DeviceControl_Output_Data_t *)data);
    break;
  case SET_DEVICE_GAIN_REPORT:
    FfbHandle_DeviceGain((USB_FFBReport_DeviceGain_Output_Data_t *)data);
    break;
  case 14:
    FfbHandle_SetCustomForce((USB_FFBReport_SetCustomForce_Output_Data_t *)data);
    break;
  default:
    break;
  }
};
