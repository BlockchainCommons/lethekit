# Blockchain Commons LetheKit

**Open source DIY hardware box for offline cryptographic tools**

![LetheKit](lethekit.jpg)

*LetheKit* is a do-it-youself platform for performing various sensitive cryptographic operations on an offline airgapped device. It uses no WiFi or Bluetooth which could leak information and contains no local storage. The external USB only charges the battery, and when the device is turned off it forgets any sensitive data stored in RAM. Thus the name Lethe (_lee-thee_), from the [mythological river](https://en.wikipedia.org/wiki/Lethe) of forgetfulness and oblivion.

## Additional Information

The following files contain everything you need to set up your LetheKit hardware and install its software.

* The [Parts List](doc/parts-list.md) describes all needed parts.
* The [Case Information](case) contains STL files for 3-D printing.
* The [Lethekit Assembly Instructions](doc/assembly.md) show how to put it together.
* The [Lethekit Installation Instructions](doc/installation.md) show how to install LetheKit in your Arduino development environment.
* The [Seedtool Installation Instructions](seedtool/doc/build.md) show to install Seedtool on your LetheKit using the Arduino IDE.
* The [Seedtool Application Instuctions](seedtool/README.md) describe how to generate and recover
[BIP-32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki) HD wallet master seeds in [BIP-39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki) and [SLIP-39](https://github.com/satoshilabs/slips/blob/master/slip-0039.md) formats.

## Status - Late Alpha

*LetheKit* is currently under active development and in the late alpha testing phase. It should not be used for production tasks until it has had further testing and auditing.

## Origin, Authors, Copyright & Licenses

Unless otherwise noted (either in this [/README.md](./README.md) or in the file's header comments) the contents of this repository are Copyright © 2020 by Blockchain Commons, LLC, and are [licensed](./LICENSE) under the [spdx:BSD-2-Clause Plus Patent License](https://spdx.org/licenses/BSD-2-Clause-Patent.html).

In most cases, the authors, copyright, and license for each file reside in header comments in the source code. The table below attempts to attribute accurately for files without such information.

This table below also establishes provenance (repository of origin, permalink, and commit id) for files included from repositories that are outside of this one. Contributors to these files are listed in the commit history for each repository, first with changes found in the commit history of this repo, then in changes in the commit history of their repo of their origin.

| File      | From                                                         | Commit                                                       | Authors & Copyright (c)                                | License                                                     |
| --------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------ | ----------------------------------------------------------- |
| Cryptosuite | [https://github.com/ksedgwic/Cryptosuite](https://github.com/ksedgwic/Cryptosuite) | ae0e3eeb4eb00c53fe235e0c34f77ade11f1982a | 2010 Peter Knight | <none> |
| bip39 | [https://github.com/ksedgwic/bip39](https://github.com/ksedgwic/bip39) | 9b8fa3c7d145c39558c2534f6cf40879477d93a1 | 2018 Chris Howe | [MIT License](https://github.com/ksedgwic/bip39/blob/master/LICENSE) |
| TRNG-for-ATSAMD51J19A-Adafruit-Metro-M4- | [https://github.com/SapientHetero/TRNG-for-ATSAMD51J19A-Adafruit-Metro-M4-](https://github.com/SapientHetero/TRNG-for-ATSAMD51J19A-Adafruit-Metro-M4-) | 17d5e36cd922ce7df8047d9c89633dca9b5ae122 | 2019 Ron Sutton | [MIT License](https://github.com/SapientHetero/TRNG-for-ATSAMD51J19A-Adafruit-Metro-M4-/blob/master/LICENSE.txt) |

### Dependencies

To build the *LetheKit* you'll need to use the following exterinal tools:

- [The Arduino IDE](https://www.arduino.cc/en/main/software)

The following internal Blockchain Commons projects are leveraged by *LetheKit*:

- [BlockchainCommons/bc-crypto-base](https://github.com/blockchaincommons/bc-crypto-base) — Well-Reviewed and Audited Cryptographic Functions for Use in Blockchain Commons Software Projects
- [BlockchainCommons/bc-shamir](https://github.com/BlockchainCommons/bc-shamir) - C Implementation of Shamir Secret Sharing for use in Blockchain Commons Software Projects
- [BlockchainCommons/bc-slip39](https://github.com/BlockchainCommons/bc-slip39) - C Implementation of SLIP-39 Shamir Secret Sharing standard.

### Derived from ...

This LetheKit project was inspired by:

- [Bitcoin 2019 Conference Tutorial](https://github.com/arcbtc/bitcoin2019conf), by [Ben Arc](https://github.com/arcbtc), which provided a fully functional starting point using similar hardware.

### Used with ...

These are other projects that work with or leverage LetheKit.

- (submit a PR to this repo list your project)

## Financial Support

LetheKit (lee-thee kit) is a project by [Blockchain Commons](https://www.blockchaincommons.com/), a not-for-profit benefit organization supporting the open web. Our work is funded entirely by donations from people like you. Every donation will be spent on building open tools & technology for blockchain & security infrastructure.

To financially support further development of *LetheKit* and other projects, please consider becoming a Patron of Blockchain Commons through ongoing individual or monthly patronage by becoming a [Sponsor](https://github.com/sponsors/BlockchainCommons) through GitHub; currently they are matching the first $5k so please do consider this option. You can also offer support with Bitcoin via our [BTCPay Server](https://btcpay.blockchaincommons.com/).

## Contributing

We encourage public contributions through issues and pull requests! Please review [CONTRIBUTING.md](./CONTRIBUTING.md) for details on our development process. All contributions to this repository require a GPG signed [Contributor License Agreement](./CLA.md).

### Credits

The following people directly contributed to this repository. You can add your name here by getting involved; the first step is to learn how to contribute from our [CONTRIBUTING.md](./CONTRIBUTING.md) documentation.

| Name              | Role                | Github                                            | Email                                 | GPG Fingerprint                                    |
| ----------------- | ------------------- | ------------------------------------------------- | ------------------------------------- | -------------------------------------------------- |
| Christopher Allen | Principal Architect | [@ChristopherA](https://github.com/@ChristopherA) | \<ChristopherA@LifeWithAlacrity.com\> | FDFE 14A5 4ECB 30FC 5D22  74EF F8D3 6C91 3574 05ED |
| Ken Sedgwick      | Project Lead        | [@ksedgwic](https://github.com/ksedgwic)          | \<ken@bonsai.com\>                  | 4695 E5B8 F781 BF85 4326 9639 BBFC E515 8602 5550  |

## Responsible Disclosure

We want to keep all our software safe for everyone. If you have discovered a security vulnerability, we appreciate your help in disclosing it to us in a responsible manner. We are unfortunately not able to offer bug bounties at this time.

We do ask that you offer us good faith and use best efforts not to leak information or harm any user, their data, or our developer community. Please give us a reasonable amount of time to fix the issue before you publish it. Do not defraud our users or us in the process of discovery. We promise not to bring legal action against researchers who point out a problem provided they do their best to follow the these guidelines.

### Reporting a Vulnerability

Please report suspected security vulnerabilities in private via email to ChristopherA@LifeWithAlacrity.com (do not use this email for support). Please do NOT create publicly viewable issues for suspected security vulnerabilities.

The following keys may be used to communicate sensitive information to developers:

| Name              | Fingerprint                                        |
| ----------------- | -------------------------------------------------- |
| Christopher Allen | FDFE 14A5 4ECB 30FC 5D22  74EF F8D3 6C91 3574 05ED |

You can import a key by running the following command with that individual’s fingerprint: `gpg --recv-keys "<fingerprint>"` Ensure that you put quotes around fingerprints that contain spaces.
