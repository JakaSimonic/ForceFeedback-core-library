# ForceFeedback-core-library

This library contains code to process FFB input, output and feature reports. It supports all effects except CustomEffects. Code can handle two axis and 10 simultaneous effects.
This library sits above your USB HID implementation. Library implements only FFB part of joystick. FFB part of HIDdescriptor is in Descriptor.txt. 

To use this library you need to:

-pass output reports to the FfbOnUsbData 

-on SET_FEATURE report id 5 use FfbOnCreateNewEffect

-on GET_FEATURE report id 6 use FfbOnPIDBlockLoad

-on GET_FEATURE report id 7 use FfbOnPIDPool

-on GET_REPORT report id 2 use FfbOnPIDStatus

-to get force for the actuator call FfbGetFeedbackValue. It ranges +/-255

-provide lib for sine and cosine functions in ffb_math.c

-provide lib for variable types (int32_t etc...) 

The condition effects have not been tested at the moment.


p.s. I have a complete FFB library inside my VjoyWrapper repository written in C#
