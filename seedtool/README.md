# Warning!  There is a problem with LetheKit/seedtool BIP39/SLIP39 interoperability.

### The following is being tracked as [Issue #38](https://github.com/BlockchainCommons/bc-lethekit/issues/38):

#### What is the problem?

The LetheKit seedtool application generates new wallets using dice
entropy to create BIP39 and SLIP39 recovery mnemonics. The BIP39 and
SLIP39 mnemonics represent valid wallets, but they are not the same
wallet.

This is a serious problem.  If both BIP39 and SLIP39 backups are
created at the same time and funds are placed in the BIP39 wallet the
SLIP39 backup will fail to recover the same wallet and the funds would
be lost.

#### Why is this the case?

Andrew Kozlik explains it well in this [GitHub comment]
(https://github.com/iancoleman/slip39/issues/1#issuecomment-563213494)

#### How can matching BIP39 and SLIP39 backups be made?

Andrew Kozlik explains (above) that in principle, SLIP39 shares which
are 59 words long could match a BIP39 mnemonic.  In practice SLIP39
shares of this length are non-standard and unlikely to be supported by
any devices or applications.  The extreme length is also significantly
painful to record and enter.

#### Impact on LetheKit/seedtool:

The desired feature of (standard) matching BIP39 and SLIP39 backups is
not possible.

The conversion of a BIP39 backup into a SLIP39 backup is not possible.

The conversion of a SLIP39 backup into a BIP39 backup is not possible.

LetheKit/seedtool can generate valid BIP39 and SLIP39 wallets but
should not allow generation of both at the same time or suggest it
can convert between the two.

LetheKit can restore SLIP39 backups, but a mechanism to export the
master secret (likely via QR code) would need to be added.

## The original (invalid) directions follow:

# Seedtool Application Instructions

The *seedtool* utility allows you to generate and recover
[BIP-32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki)
HD wallet master seeds using
[BIP-39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki)
and
[SLIP-39](https://github.com/satoshilabs/slips/blob/master/slip-0039.md)
formats.

## Compile and Upload Instruction

Please see the [Seedtool Installation Instructions](doc/build.md).

## "No Seed" Functions

There are three ways to insert a key into the *seedtool*:

![No Seed Menu](doc/images/no-seed.png)

### Key Generation with Dice

By rolling dice and typing the values, you can gather enough auditable entropy to generate a secure master seed.  Rolling 50 dice gathers
roughly 128 bits of entropy.

![Generate Seed](doc/images/generate-seed.png)

### BIP-39 Key Recovery

You can insert a key into the *seedtool* by entering its BIP-39
recovery mnemonic passphrase.  From there you can generate SLIP-39
shares.

![BIP-39 Restore](doc/images/bip39-restore.png)

### SLIP-39 Key Recovery

If you possess enough shares of a SLIP-39 set, you can recover the
master seed with *seedtool*.  From there you can generate the BIP-39
mnemonic passphrase which will allow you to use it with most wallets.

![SLIP-39 Recovery Menu](doc/images/slip39-restore-menu.png) ![SLIP-39 Share Entry](doc/images/slip39-share-restore.png)

## Functions with a Seed

Once you have a seed through any of the prior flows, you can create
BIP-39 and SLIP-39 mnemonic passphrases.

![Seed Present Menu](doc/images/seed-present.png)

### BIP-39 Generation

The BIP-39 phrase is displayed and can be backed up in a secure
offline fashion (e.g., hammering into metal).

![BIP-39 View](doc/images/bip39-view.png)

### SLIP-39 Generation

SLIP-39 requires some configuration choices to determine total number of shares
and the required number of shares present to recover.

![SLIP-39 Generation Config](doc/images/config-slip39.png) ![SLIP-39 Share View](doc/images/slip39-share-view.png)

## Common Workflows

There are several common key management workflows that *seedtool* is
useful for.

### Generate a New Wallet

By using dice to generate a BIP-39 mnemonic passphrase, you can avoid
trusting a particular hardware vendor's hardware.
[See: RNG subversion](https://en.wikipedia.org/wiki/Random_number_generator_attack#RNG_subversion)

To execute this flow with *seedtool*:
1. Generate a new master seed using 50+ die rolls.
2. Record your master BIP-39 backup in a secure manner (hammer into metal).
   This BIP-39 backup may be restored into most other wallets for common use.
3. Optionally create a SLIP-39 sharded backup set which can serve as
   long-term cold storage.
   
### Generate a SLIP-39 Backup Set for an Existing Wallet

If you've already got a wallet in use, and have the BIP-39 backup for
it, you can generate a SLIP-39 backup set as well:
1. Restore your seed in *seedtool* using your BIP-39 backup.
2. Create a SLIP-39 sharded backup.

### Recover a Master Seed from SLIP-39 Backup Shards

If you need to recover your master seed and have enough shards from a
SLIP-39 backup set:
1. Recover the master seed from the SLIP-39 shards.
2. Create a BIP-39 mnemonic which may then be directly restored into
   most wallets for use.
