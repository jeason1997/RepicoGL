#ifndef DEBUGGER_HH
#define DEBUGGER_HH

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "HardwareSerial.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    extern HardwareSerial* fmt;

#ifdef __cplusplus
}
#endif

#endif
