## Seedtool Application Instructions

### One Time Setup

Enable the generation of the `gitrevision.h` file.  This only needs to
be done once, but is safe to do anytime you like.

    # Must be run in the seedtool directory
    ./enable-gitrevision-hooks.sh

### Install Arduino IDE

Start with the [SparkFun Software Setup
Directions](https://learn.sparkfun.com/tutorials/samd51-thing-plus-hookup-guide/setting-up-the-arduino-ide)
to install the IDE and establish basic functionality.

Next, from "Manage Libraries" install:
```
* GxEPD2
* Keypad
```

Install pyserial:

    pip install --user pyserial


### Install Blockchain Commons Components

```
cd ~/Arduino/libraries

# Install Cryptosuite
git clone git@github.com:ksedgwic/Cryptosuite.git

# Install bip39
git clone git@github.com:ksedgwic/bip39.git

# Install bc-crypto-base
git clone git@github.com:BlockchainCommons/bc-crypto-base.git

# Install bc-shamir
git clone git@github.com:BlockchainCommons/bc-shamir.git

# Install bc-slip39
git clone --recurse-submodules git@github.com:BlockchainCommons/bc-slip39.git
(cd bc-slip39/src && ln -s ../../bc-crypto-base/src bc-crypto-base)
(cd bc-slip39/src && ln -s ../../bc-shamir/src bc-shamir)
```

### Add Libraries

Restart the Arduino IDE.

From the Arduino IDE, menu "Sketch" -> "Include Library"
* Select Cryptosuite
* Select bip39
* Select bc-crypto-base
* Select bc-shamir
* Select bc-slip39
