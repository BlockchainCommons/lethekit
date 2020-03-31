* Add LetheKit specific programming instructions:
  1. Open case
  2. Disconnect battery
  3. Connect device to dev computer USB port.
  4. Double press reset button
  5. Confirm port in Arduino IDE
  6. Upload

* Need better diagnsotics for BIP39 and SLIP39 restore operations.

* Update to bc_bip32 (waiting for Wolf).

* Remove the ESP32 code from lethe.ino?

* Should combine_slip39_shares() take globals as args?  Inconsistent ...

* Structure the globals in seedtool.

* Break seedtool into MVCish components.

* Give faster feedback on SLIP39 generate, clear the screen first,
  then generate.

* Find a new supplier for keypads, costs $150 to ship $20 worth ...


#### Semantic Versioning Reminder

Given a version number MAJOR.MINOR.PATCH, increment the:
* MAJOR version when you make incompatible API changes,
* MINOR version when you add functionality in a backwards compatible manner
* PATCH version when you make backwards compatible bug fixes.
