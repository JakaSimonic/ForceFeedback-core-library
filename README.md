# ForceFeedback-core-library

This library contains code to process FFB input, output and feature reports. It supports all effects except CustomEffects.
This library sits above your USB HID implementation.  

To use this library you need to:
-pass output reports to the FfbOnUsbData 
-pass data to  FfbOnCreateNewEffect on SET_FEATURE_REPORT ID 5
-on GET_FEATURE report id 6 use FfbOnPIDBlockLoad
-on GET_FEATURE report id 7 use FfbOnPIDPool
-on GET_REPORT report id 2 use FfbOnPIDStatus
