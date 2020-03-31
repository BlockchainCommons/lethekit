## Seedtool Application Instructions

### One Time Setup

Enable the generation of the `gitrevision.h` file.  This only needs to
be done once, but is safe to do anytime you like.

    # Must be run in the seedtool directory
    ./enable-gitrevision-hooks.sh

### Install Lethe Kit in your Arduino Sketchbook

[Lethe Kit Installation Instructions](../doc/installation.md)

### Add Libraries

Launch the Arduino IDE in the `seedtool` subdirectory:

    arduino ./
    
Open `Tools` -> `Manage Libraries...`, install the following:
* `GxEPD2`
* `Adafruit GFX Library`
* `Keypad`

### Build seedtool

Connect a Lethe Kit hardware device to an appropriate USB port.

Press the "Upload" button. The `seedtool` application should be built
and uploaded to your device.
