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
#ifndef FFBENGINE_h
#define FFBENGINE_h

#include "HIDReportType.h"
#include "FfbReportHandler.h"
#include "UserInput.h"

class FfbEngine
{
public:
  FfbEngine(FfbReportHandler &reporthandler, UserInput &uIn, uint64_t (*)(void), int32_t (*)(float, int8_t, int8_t) = nullptr);
  ~FfbEngine();

  void ForceCalculator(int32_t[NUM_AXES]);
  float ConstantForceCalculator(const TEffectState &effect);
  float RampForceCalculator(const TEffectState &effect, float elapsedTime);
  void ConditionForceCalculator(const TEffectState &effect, const int32_t metric[NUM_AXES], float outForce[NUM_AXES]);
  float PeriodiceForceCalculator(uint8_t effectType, const TEffectState &effect, float elapsedTime);
  float GetEnvelope(const USB_FFBReport_SetEnvelope_Output_Data_t &effect, uint32_t elapsedTime, uint16_t duration);
  bool IsEffectPlaying(const TEffectState &effect, uint64_t time);

private:
  FfbReportHandler &ffbReportHandler;
  UserInput &axisPosition;
  uint64_t (*getTimeMilli)(void);
  int32_t (*forceHook)(float forceValue, int8_t effect, int8_t axisIndex);
};

#endif
