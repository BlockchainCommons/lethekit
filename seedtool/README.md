## Seedtool Application Instructions

### One Time Setup

Enable the generation of the `gitrevision.h` file.  This only needs to
be done once, but is safe to do anytime you like.

    # Must be run in the seedtool directory
    ./enable-gitrevision-hooks.sh

### Install Lethe Kit in your Arduino Sketchbook

[Lethe Kit Installation Instructions](../doc/installation.md)


### Add Libraries

Restart the Arduino IDE:

From the Arduino IDE, menu "Sketch" -> "Include Library"
* Select Cryptosuite
* Select bip39
* Select bc-crypto-base
* Select bc-shamir
* Select bc-slip39

### Build seedtool

Connect a Lethe Kit hardware device to an appropriate USB port
and start the Arduino IDE in the seedtool subdirectory:

    cd $LETHEKIT_ROOT/seedtool
    arduno ./

Press the "Upload" button. The `seedtool` application should be built
and uploaded to your device.
