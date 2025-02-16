#ifndef HID_TYPES_EXT
#define HID_TYPES_EXT
#include "../../HIDReportType.h"

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
        uint8_t directionX,
        uint8_t directionY,
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
        uint16_t loopCount)
    {
        this->reportId = SET_EFFECT_OPERATION_REPORT;
        this->effectBlockIndex = effectBlockIndex;
        this->operation = operation;
        this->loopCount = loopCount;
    }
};

#endif // HID_TYPES_EXT