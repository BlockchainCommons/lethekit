// Copyright © 2020 Blockchain Commons, LLC

#ifndef HARDWARE_H
#define HARDWARE_H

#include <GxEPD2_BW.h>
#include <Keypad.h>

// Around April 2020 Waveshare started shipping a new version of the
// "Waveshare 200x200, 1.54inch E-Ink display module".  The new
// versions have "Rev2.1" printed under the title on the circuit side
// of the display.  This new version requires a different driver
// module from GxEPD2.
//
// If you have an *older* display (doesn't have "Rev2.1") uncomment
// the following line:
//
// #define LEGACY_WAVESHARE

#if !defined(LEGACY_WAVESHARE)
#define EPD_DRIVER GxEPD2_154_D67
#else
#define EPD_DRIVER GxEPD2_154
#endif

// This is hard to hide/encapsulate.
extern GxEPD2_BW<EPD_DRIVER, EPD_DRIVER::HEIGHT> g_display;

void hw_setup();
void hw_green_led(int value);

extern "C" {
void hw_random_buffer(uint8_t *buf, size_t len);
}

#endif // HARDWARE_H
