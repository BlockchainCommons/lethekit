* Add LetheKit specific programming instructions:
  1. Open case
  2. Disconnect battery
  3. Connect device to dev computer USB port.
  4. Double press reset button
  5. Confirm port in Arduino IDE
  6. Upload

* Need better diagnsotics for BIP39 and SLIP39 restore operations.

* Add negative tests to selftest.

* Update to bc_bip32 (waiting for Wolf).

* Remove the ESP32 code from lethe.ino?

* Find a new supplier for keypads, costs $150 to ship $20 worth ...

* Can we make WordListState::wordndx a uint16_t? Would streamline.

#### Semantic Versioning Reminder

Given a version number MAJOR.MINOR.PATCH, increment the:
* MAJOR version when you make incompatible API changes,
* MINOR version when you add functionality in a backwards compatible manner
* PATCH version when you make backwards compatible bug fixes.

#### Punchlist

Crib from https://github.com/blockchaincommons/fullynoded-2:
  
* financial support section: 

* where should testers focus

* Where to send questions (you directly or through issues?

8d969eef6ecad3c29a3a629280e686cf
8d969eef6ecad3c29a3a629280e686cf

1. dummy data
2. fix assert
3. fix bip39 restore
