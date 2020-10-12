// Copyright © 2020 Blockchain Commons, LLC

#ifndef HARDWARE_H
#define HARDWARE_H

#include <GxEPD2_GFX.h>
#include <Keypad.h>

// This is hard to hide/encapsulate.
extern GxEPD2_GFX *g_display;

void hw_setup();
void hw_green_led(int value);

extern "C" {
void hw_random_buffer(uint8_t *buf, size_t len);
void random_buffer(uint8_t *buf, size_t len, void * p = NULL);
}

#endif // HARDWARE_H
