#ifndef HID_TYPES_EXT
#define HID_TYPES_EXT
#include "HIDReportType.h"

struct DeviceGain_Ext : public USB_FFBReport_DeviceGain_Output_Data_t
{
    DeviceGain_Ext(
        uint8_t gain)
    {
        this->reportId = SET_DEVICE_GAIN_REPORT;
        this->gain = gain;
    }
};

struct DeviceControl_Ext : public USB_FFBReport_DeviceControl_Output_Data_t
{
    DeviceControl_Ext(
        uint8_t control)
    {
        this->reportId = SET_DEVICE_CONTROL_REPORT;
        this->control = control;
    }
};

struct SetCondition_Ext : public USB_FFBReport_SetCondition_Output_Data_t
{
    SetCondition_Ext(
        uint8_t effectBlockIndex,
        uint8_t parameterBlockOffset,
        int16_t cpOffset,
        uint16_t positiveCoefficient,
        uint16_t negativeCoefficient,
        uint16_t positiveSaturation,
        uint16_t negativeSaturation,
        uint16_t deadBand)
    {
        this->reportId = SET_CONDITION_REPORT;
        this->effectBlockIndex = effectBlockIndex;
        this->parameterBlockOffset = parameterBlockOffset;
        this->cpOffset = cpOffset;
        this->positiveCoefficient = positiveCoefficient;
        this->negativeCoefficient = negativeCoefficient;
        this->positiveSaturation = positiveSaturation;
        this->negativeSaturation = negativeSaturation;
        this->deadBand = deadBand;
    }
};

struct BlockFree_Ext : public USB_FFBReport_BlockFree_Output_Data_t
{
    BlockFree_Ext(uint8_t effectBlockIndex = 0xFF)
    {
        this->reportId = SET_BLOCK_FREE_REPORT;
        this->effectBlockIndex = effectBlockIndex;
    }
};

struct SetRampForce_Ext : public USB_FFBReport_SetRampForce_Output_Data_t
{
    SetRampForce_Ext(uint8_t effectBlockIndex, uint16_t startMagnitude, uint16_t endMagnitude)
    {
        this->reportId = SET_RAMP_FORCE_REPORT;
        this->effectBlockIndex = effectBlockIndex;
        this->startMagnitude = startMagnitude;
        this->endMagnitude = endMagnitude;
    }
};

struct SetPeriodic_Ext : public USB_FFBReport_SetPeriodic_Output_Data_t
{
    SetPeriodic_Ext(uint8_t effectBlockIndex, uint16_t magnitude, int16_t offset, uint16_t phase, uint16_t period)
    {
        this->reportId = SET_PERIODIC_REPORT;
        this->effectBlockIndex = effectBlockIndex;
        this->magnitude = magnitude;
        this->offset = offset;
        this->phase = phase;
        this->period = period;
    }
};

struct SetEnvelope_Ext : public USB_FFBReport_SetEnvelope_Output_Data_t
{
    SetEnvelope_Ext(uint8_t effectBlockIndex, int16_t attackLevel, int16_t fadeLevel, int16_t attackTime, int16_t fadeTime)
    {
        this->reportId = SET_ENVELOPE_REPORT;
        this->effectBlockIndex = effectBlockIndex;
        this->attackLevel = attackLevel;
        this->fadeLevel = fadeLevel;
        this->attackTime = attackTime;
        this->fadeTime = fadeTime;
    }
};

struct SetConstantForce_Ext : public USB_FFBReport_SetConstantForce_Output_Data_t
{
    SetConstantForce_Ext(uint8_t effectBlockIndex, int16_t magnitude)
    {
        this->reportId = SET_CONSTANT_FORCE_REPORT;
        this->effectBlockIndex = effectBlockIndex;
        this->magnitude = magnitude;
    }
};

struct SetEffect_Ext : public USB_FFBReport_SetEffect_Output_Data_t
{
    SetEffect_Ext(
        uint8_t effectBlockIndex,
        uint8_t effectType,
        uint16_t duration,
        uint16_t triggerRepeatInterval,
        uint16_t samplePeriod,
        uint8_t gain,
        uint8_t triggerButton,
        uint8_t enableAxis,
        uint16_t directionX,
        uint16_t directionY,
        uint16_t startDelay)
    {
        this->reportId = SET_EFFECT_REPORT;
        this->effectBlockIndex = effectBlockIndex;
        this->effectType = effectType;
        this->duration = duration;
        this->triggerRepeatInterval = triggerRepeatInterval;
        this->samplePeriod = samplePeriod;
        this->gain = gain;
        this->triggerButton = triggerButton;
        this->enableAxis = enableAxis;
        this->directionX = directionX;
        this->directionY = directionY;
        this->startDelay = startDelay;
    }
};

struct EffectOperation_Ext : public USB_FFBReport_EffectOperation_Output_Data_t
{
    EffectOperation_Ext(
        uint8_t effectBlockIndex,
        uint8_t operation,
        uint16_t loopCount = 0)
    {
        this->reportId = SET_EFFECT_OPERATION_REPORT;
        this->effectBlockIndex = effectBlockIndex;
        this->operation = operation;
        this->loopCount = loopCount;
    }
};

#endif // HID_TYPES_EXT