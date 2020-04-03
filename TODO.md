* Add power-on-self test diagnostics to the screen.

* Need better diagnsotics for BIP39 and SLIP39 restore operations.

* Add negative tests to selftest.

* Update to bc_bip32 (waiting for Wolf).

* Remove the ESP32 code from lethe.ino?

* Find a new supplier for keypads, costs $150 to ship $20 worth ...

* Can we make WordListState::wordndx a uint16_t? Would streamline.

* Add compatibility instructions for https://iancoleman.io/bip39/

* Add Security Considerations section to README.md, in particular talk about custody of device, evil maid attack, and tamper evident measures.

#### Semantic Versioning Reminder

Given a version number MAJOR.MINOR.PATCH, increment the:
* MAJOR version when you make incompatible API changes,
* MINOR version when you add functionality in a backwards compatible manner
* PATCH version when you make backwards compatible bug fixes.

#### Punchlist

Mark ksedgwic/seedtool (my old repo) as deprecated, point to lethekit.
