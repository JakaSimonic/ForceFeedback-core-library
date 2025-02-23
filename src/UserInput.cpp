
/*
  Force Feedback Joystick
  Axis Position helper.
  Copyright 2025  Jaka Simonic    (telesimke [at] gmail [dot] com)
  MIT License.
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
#include "UserInput.h"

UserInput::UserInput()
{
}

void UserInput::UpdatePosition(const int32_t newPosition[NUM_AXES])
{
  for (uint8_t i = 0; i < NUM_AXES; ++i)
  {
    int32_t tempSpeed = newPosition[i] - metrics[position][i];
    int32_t tempAcc = tempSpeed - metrics[speed][i];

    metrics[position][i] = newPosition[i];
    metrics[speed][i] = tempSpeed;
    metrics[acceleration][i] = tempAcc;
  }
}

void UserInput::UpdateMetrics(const int32_t newPosition[NUM_AXES], const int32_t newSpeed[NUM_AXES], const int32_t newAcc[NUM_AXES])
{
  for (uint8_t i = 0; i < NUM_AXES; ++i)
  {
    metrics[position][i] = newPosition[i];
    metrics[speed][i] = newSpeed[i];
    metrics[acceleration][i] = newAcc[i];
  }
}

void UserInput::UpdateButtons(int8_t buttons)
{
  buttonsState = buttons;
}

const int32_t *UserInput::GetMetric(Metric m)
{
  return (const int32_t *)metrics[m];
}

uint8_t UserInput::GetButtons()
{
  return buttonsState;
}
