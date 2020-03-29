## Arduino IDE Setup Instructions

### Install the Arduino IDE

https://www.arduino.cc/en/main/software

### Enable Library Dependencies

FIXME - Figure out which libraries we are dependent on.

### Install Blockchain Commons Components

```
cd ~/Arduino/libraries

# Install bc-bip39
git clone git@github.com:BlockchainCommons/bc-bip39.git

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

From the Arduino IDE, menu "Sketch" -> "Include Library"
* Select bc-crypto-base
* Select bc-shamir
* Select bc-slip39

