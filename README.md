# ForceFeedback-core-library

This library contains code to process FFB input, output and feature reports. It supports all effects except CustomEffects. Code can handle two axis.
This library sits above your USB HID implementation. Library implements only FFB part of joystick. FFB part of HIDdescriptor is in Descriptor.txt. 

To use this library you need to:

-pass output reports to the FfbOnUsbData 

-pass data to  FfbOnCreateNewEffect on SET_FEATURE_REPORT ID 5

-on GET_FEATURE report id 6 use FfbOnPIDBlockLoad

-on GET_FEATURE report id 7 use FfbOnPIDPool

-on GET_REPORT report id 2 use FfbOnPIDStatus

-to get actuator force you need to call FfbGetFeedbackValue

The condition effect have not been tested at the moment.
