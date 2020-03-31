* Switch the gitversion to straight `git describe --tags --long --always`

* Need better diagnsotics for BIP39 and SLIP39 restore operations.

* Update to bc_bip32 (waiting for Wolf).

* Remove the ESP32 code from lethe.ino?

* Should combine_slip39_shares() take globals as args?  Inconsistent ...

* Structure the globals in seedtool.

* Break seedtool into MVCish components.

* Give faster feedback on SLIP39 generate, clear the screen first,
  then generate.

* Find a new supplier for keypads, costs $150 to ship $20 worth ...

Other sources:

* Arduino base software/runtime.

* SparkFun's "Cores"

* GxEPD2 
  https://github.com/ZinggJM/GxEPD
  GNU General Public License v3.0

* Adafruit GFX Library
  https://github.com/adafruit/Adafruit-GFX-Library
  https://github.com/adafruit/Adafruit-GFX-Library/blob/master/license.txt
  
* Keypad
  https://playground.arduino.cc/Code/Keypad/
  GNU General Public License v3.0

* TRNG-for-ATSAMD51J19A-Adafruit-Metro-M4-
  https://github.com/SapientHetero/TRNG-for-ATSAMD51J19A-Adafruit-Metro-M4-
  MIT License
