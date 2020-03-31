// Copyright © 2020 Blockchain Commons, LLC

#include <assert.h>
#include <stdarg.h>

#include "hardware.h"
#include "selftest.h"
#include "seed.h"
#include "userinterface.h"

#include "gitrevision.h"

void setup() {
    hw_setup();

    selftest();

    seed_reset_state();
    ui_reset_state();
    
    Serial.println("seedtool starting");
}

void loop() {
    ui_dispatch();
}
