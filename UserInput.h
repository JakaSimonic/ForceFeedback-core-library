
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
#ifndef USERINPUT_h
#define USERINPUT_h

#include <stdint.h>
#include "HIDReportType.h"

class UserInput
{
public:
    UserInput();
    void UpdatePosition(int32_t[NUM_AXES]);
    void UpdateMetrics(int32_t[NUM_AXES], int32_t[NUM_AXES], int32_t[NUM_AXES]);
    void UpdateButtons(int8_t);
    uint8_t GetButtons();

    enum Metric
    {
        position,
        speed,
        acceleration,
        metricsCount
    };
    const int32_t *GetMetric(Metric);

private:
    int32_t metrics[metricsCount][NUM_AXES] = {0};
    uint8_t buttonsState = 0;
};

#endif