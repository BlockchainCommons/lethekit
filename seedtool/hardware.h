// Copyright © 2020 Blockchain Commons, LLC

#ifndef HARDWARE_H
#define HARDWARE_H

#include <GxEPD2_BW.h>
#include <Keypad.h>

// FIXME - can we hide g_display?
extern GxEPD2_BW<GxEPD2_154, GxEPD2_154::HEIGHT> g_display;

void hw_setup();
void hw_green_led(int value);

extern "C" {
void hw_random_buffer(uint8_t *buf, size_t len);
}

#endif // HARDWARE_H
