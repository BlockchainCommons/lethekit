// Copyright © 2020 Blockchain Commons, LLC

#include "hardware.h"
#include "selftest.h"
#include "seed.h"
#include "userinterface.h"

#include "gitrevision.h"

void setup() {

    #if 0
    hw_setup();
    #endif

    // To see the serial debugging output from the power-on-self-test
    // uncomment this delay to give you a chance to get the serial
    // monitor launched before the tests run.
    //
    delay(5000);
    
    selftest();

    #if 0
    
    seed_reset_state();
    ui_reset_state();

    #endif
    
    Serial.println("seedtool starting");
}

void loop() {
#if 0
    ui_dispatch();
#endif    
}
