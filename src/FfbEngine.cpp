/*
  Force Feedback Joystick Math
  Joystick model specific code for calculating force feedback.
  Copyright 2016  Jaka Simonic
  Copyright 2025  Jaka Simonic
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

#include <math.h>
#include "FfbEngine.h"
#include "HIDReportType.h"

FfbEngine::FfbEngine(
    FfbReportHandler &reporthandler,
    UserInput &uIn,
    uint64_t (*pTime)(void),
    int32_t (*fHook)(float, int8_t, int8_t)) : ffbReportHandler{reporthandler},
                                               axisPosition{uIn},
                                               getTimeMilli{pTime},
                                               forceHook{fHook}
{
}

FfbEngine::~FfbEngine()
{
}

float FfbEngine::ConstantForceCalculator(const TEffectState &effect)
{
  return effect.parameters[TYPE_SPECIFIC_BLOCK_OFFSET_1].constant.magnitude;
}

float FfbEngine::RampForceCalculator(const TEffectState &effect, float elapsedTime)
{
  const USB_FFBReport_SetRampForce_Output_Data_t &ramp = effect.parameters[TYPE_SPECIFIC_BLOCK_OFFSET_1].ramp;

  float tempForce = ramp.startMagnitude + elapsedTime * (ramp.endMagnitude - ramp.startMagnitude) / effect.block.duration;
  return tempForce;
}

float FfbEngine::PeriodiceForceCalculator(uint8_t effectType, const TEffectState &effect, float elapsedTime)
{
  const USB_FFBReport_SetPeriodic_Output_Data_t &periodic = effect.parameters[TYPE_SPECIFIC_BLOCK_OFFSET_1].periodic;

  float offset = periodic.offset;
  float magnitude = periodic.magnitude;
  float phase = periodic.phase;
  uint32_t period = periodic.period;

  float phaseNormalized = phase / USB_MAX_PHASE;
  uint32_t elapsedPlusPhaseTime = phaseNormalized * period + elapsedTime;
  uint32_t remainder = elapsedPlusPhaseTime % period;

  float tempForce = 0;
  switch (effectType)
  {
  case USB_EFFECT_SQUARE:
  {
    if (remainder >= (period / 2))
      tempForce = -magnitude;
    else
      tempForce = magnitude;
    tempForce += offset;
  }
  break;
  case USB_EFFECT_SINE:
  {
    float angle = 2 * M_PI * (elapsedTime / period + phaseNormalized);
    tempForce = sin(angle) * magnitude;
    tempForce += offset;
  }
  break;
  case USB_EFFECT_TRIANGLE:
  {
    float slope = 4 * magnitude / period;
    const uint32_t phaseOffset = period / 4;
    uint32_t offsetRemainder = (remainder + phaseOffset) % period;
    if (offsetRemainder >= (period / 2))
      tempForce = slope * (period - offsetRemainder);
    else
      tempForce = slope * offsetRemainder;
    tempForce -= magnitude;
    tempForce += offset;
  }
  break;
  case USB_EFFECT_SAWTOOTHUP:
  case USB_EFFECT_SAWTOOTHDOWN:
  {
    float slope = magnitude / period;
    if (effectType == USB_EFFECT_SAWTOOTHDOWN)
      tempForce = slope * (period - remainder);
    else
      tempForce = slope * remainder;
    tempForce += offset;
  }
  break;
  default:
    return 0;
  }

  return tempForce;
}

int32_t ApplyCondition(int32_t metric, uint8_t gain, const USB_FFBReport_SetCondition_Output_Data_t &condition)
{
  uint16_t deadBand = condition.deadBand;
  int16_t cpOffset = condition.cpOffset;
  uint16_t negativeCoefficient = condition.negativeCoefficient;
  int16_t negativeSaturation = -condition.negativeSaturation;
  uint16_t positiveSaturation = condition.positiveSaturation;
  uint16_t positiveCoefficient = condition.positiveCoefficient;

  float tempForce = 1.0 / USB_AXIS_MAX_ABSOLUTE;

  if (metric < (cpOffset - deadBand))
  {
    tempForce *= (metric - (cpOffset - deadBand)) * negativeCoefficient;
    if (tempForce < negativeSaturation)
      tempForce = negativeSaturation;
  }
  else if (metric > (cpOffset + deadBand))
  {
    tempForce *= (metric - (cpOffset + deadBand)) * positiveCoefficient;
    if (tempForce > positiveSaturation)
      tempForce = positiveSaturation;
  }

  return -tempForce;
}

void FfbEngine::ConditionForceCalculator(const TEffectState &effect, const int32_t metric[NUM_AXES], float outForce[NUM_AXES])
{
  uint8_t gain = (float)effect.block.gain;
  uint8_t enableAxis = effect.block.enableAxis;

  if (enableAxis & DIRECTION_ENABLE)
  {
    const USB_FFBReport_SetCondition_Output_Data_t &condition = effect.parameters[TYPE_SPECIFIC_BLOCK_OFFSET_1].condition;

    float metricComponent = 0;
    for (uint8_t i = 0; i < NUM_AXES; ++i) // size of metric's vector component in the direction of condition effect
    {
      metricComponent += metric[i] * effect.directionUnitVec[i];
    }

    float tempForce = ApplyCondition(metricComponent, gain, condition);

    for (uint8_t i = 0; i < NUM_AXES; ++i) // split the force to components in axis directions
    {
      outForce[i] = tempForce * effect.directionUnitVec[i];
    }
    return;
  }

  for (uint8_t i = 0; i < NUM_AXES; ++i)
  {
    outForce[i] = 0;
    if (!((enableAxis >> i) & 0x01))
      continue;

    USB_FFBReport_SetCondition_Output_Data_t condition = effect.parameters[i].condition;
    outForce[i] = ApplyCondition(metric[i], gain, condition);
  }
}

void FfbEngine::ForceCalculator(int32_t ffbForce[NUM_AXES])
{
  if (ffbReportHandler.devicePaused)
  {
    for (uint8_t i = 0; i < NUM_AXES; ++i)
    {
      ffbForce[i] = 0;
    }
    return;
  }

  const TEffectState *effectStates = ffbReportHandler.GetEffectStates();

  float forceSum[NUM_AXES] = {0};
  uint64_t time = getTimeMilli();

  for (uint8_t idx = 0; idx < MAX_EFFECTS; ++idx)
  {
    const TEffectState &effect = effectStates[idx];

    if (IsEffectPlaying(effect, time))
    {
      uint8_t effectType = effect.block.effectType;
      uint16_t duration = effect.block.duration;
      uint32_t elapsedTime = time - effect.startTime;
      uint8_t gain = effect.block.gain;

      float force = 0;
      float forceCondition[NUM_AXES] = {0};

      switch (effectType)
      {
      case USB_EFFECT_CONSTANT:
        force = ConstantForceCalculator(effect);
        break;
      case USB_EFFECT_RAMP:
        force = RampForceCalculator(effect, elapsedTime);
        break;
      case USB_EFFECT_SQUARE:
      case USB_EFFECT_SINE:
      case USB_EFFECT_TRIANGLE:
      case USB_EFFECT_SAWTOOTHDOWN:
      case USB_EFFECT_SAWTOOTHUP:
        force = PeriodiceForceCalculator(effectType, effect, elapsedTime);
        break;
      case USB_EFFECT_SPRING:
        ConditionForceCalculator(effect, axisPosition.GetMetric(UserInput::position), forceCondition);
        break;
      case USB_EFFECT_FRICTION:
      case USB_EFFECT_DAMPER:
        ConditionForceCalculator(effect, axisPosition.GetMetric(UserInput::speed), forceCondition);
        break;
      case USB_EFFECT_INERTIA:
        ConditionForceCalculator(effect, axisPosition.GetMetric(UserInput::acceleration), forceCondition);
        break;
      case USB_EFFECT_CUSTOM:
      default:
        continue;
      }

      switch (effectType)
      {
      case USB_EFFECT_CONSTANT:
      case USB_EFFECT_RAMP:
      case USB_EFFECT_SQUARE:
      case USB_EFFECT_SINE:
      case USB_EFFECT_TRIANGLE:
      case USB_EFFECT_SAWTOOTHDOWN:
      case USB_EFFECT_SAWTOOTHUP:
        if (effect.envelopeParameter)
        {
          const USB_FFBReport_SetEnvelope_Output_Data_t &envelope = effect.parameters[TYPE_SPECIFIC_BLOCK_OFFSET_2].envelope;
          force *= GetEnvelope(envelope, elapsedTime, duration);
        }
        force *= gain;
        force /= USB_MAX_GAIN;
        for (uint8_t i = 0; i < NUM_AXES; ++i)
        {
          if (effect.block.enableAxis >> i & 0x01)
          {
            force *= effect.directionUnitVec[i];

            if (forceHook != nullptr)
              force = forceHook(force, effectType, i);

            forceSum[i] += force;
          }
        }
        break;
      case USB_EFFECT_SPRING:
      case USB_EFFECT_FRICTION:
      case USB_EFFECT_DAMPER:
      case USB_EFFECT_INERTIA:
        for (uint8_t i = 0; i < NUM_AXES; ++i)
        {
          forceCondition[i] *= gain;
          forceCondition[i] /= USB_MAX_GAIN;

          if (forceHook != nullptr)
            force = forceHook(force, effectType, i);

          forceSum[i] += forceCondition[i];
        }
        break;
      case USB_EFFECT_CUSTOM:
      default:
        continue;
      }
    }
  }

  for (uint8_t i = 0; i < NUM_AXES; ++i)
  {
    forceSum[i] *= ffbReportHandler.deviceGain;
    forceSum[i] /= USB_MAX_GAIN;

    ffbForce[i] = forceSum[i];
  }
}

float FfbEngine::GetEnvelope(const USB_FFBReport_SetEnvelope_Output_Data_t &envelope, uint32_t elapsedTime, uint16_t duration)
{
  int32_t attackLevel = envelope.attackLevel;
  int32_t fadeLevel = envelope.fadeLevel;
  float envelopeValue = USB_MAX_MAGNITUDE;
  int32_t attackTime = envelope.attackTime;
  int32_t fadeTime = envelope.fadeTime;

  if (elapsedTime < attackTime)
  {
    float height = (USB_MAX_MAGNITUDE - attackLevel);
    float slope = height / attackTime;
    envelopeValue = slope * elapsedTime + attackLevel;
    return envelopeValue / USB_MAX_MAGNITUDE;
  }

  if (duration == USB_DURATION_INFINITE)
  {
    return 1.0;
  }

  if (elapsedTime >= (duration - fadeTime))
  {
    float height = (USB_MAX_MAGNITUDE - fadeLevel);
    float slope = height / fadeTime;
    envelopeValue = slope * (duration - elapsedTime) + fadeLevel;
    return envelopeValue / USB_MAX_MAGNITUDE;
  }

  return 1.0;
}

bool IsTriggerEffectPlaying(TEffectState &effect, uint8_t buttonState, uint64_t time)
{
  int64_t elapsedTime = time - effect.startTime;
  uint8_t buttonIdx = effect.block.triggerButton - 1;
  bool buttonPressed = ((buttonState >> buttonIdx) & 0x01);
  if (!buttonPressed)
  {
    effect.triggerButtonLatch = false;
    return false;
  }
  else
  {
    if (!effect.triggerButtonLatch)
    {
      effect.startTime = time;
      effect.triggerButtonLatch = true;
      return true;
    }
    else
    {
      if (elapsedTime < effect.block.duration)
        return true;

      if (elapsedTime < (effect.block.duration + effect.block.triggerRepeatInterval))
        return false;

      effect.startTime = time;
      return true;
    }
  }
}

bool FfbEngine::IsEffectPlaying(const TEffectState &effect, uint64_t time)
{
  if (!(effect.state & MEFFECTSTATE_PLAYING))
    return false;

  if (effect.block.triggerButton != USB_NO_TRIGGER_BUTTON)
  {
    return IsTriggerEffectPlaying(const_cast<TEffectState &>(effect), axisPosition.GetButtons(), time);
  }

  int64_t elapsedTime = time - effect.startTime;
  if (elapsedTime < 0)
    return false;

  if ((effect.block.duration != USB_DURATION_INFINITE) && (elapsedTime >= effect.block.duration))
    return false;

  return true;
}
