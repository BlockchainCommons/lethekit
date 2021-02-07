// Copyright © 2020 Blockchain Commons, LLC

#include "hardware.h"
#include "selftest.h"
#include "seed.h"
#include "userinterface.h"
#include "util.h"

#include "gitrevision.h"

void setup() {

    hw_setup();

    // To see the serial debugging output from the power-on-self-test
    // uncomment this delay to give you a chance to get the serial
    // monitor launched before the tests run.
    //
    // delay(5000);

    ui_reset_into_state(SELF_TEST);

    Serial.println("seedtool starting");
}

void loop() {
    ui_dispatch();
}
