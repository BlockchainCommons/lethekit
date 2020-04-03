## LetheKit Installation Instructions

These instructions describe how to install LetheKit into your local
development environment.  We presume you already have the Arduino IDE
installed, which will allow you upload software to your **SAMD51 Thing Plus**, contained in your LetheKit hardware.

LetheKit provides a collection of security modules (libraries) that
work well together.  You install them into your local Arduinio
development environment and then build the provided sample applications or
develop your own applications before uploading them to your SAMD51.

### Install the Arduino IDE

After installing the Arduino IDE, run it once and let it create the
default sketchbook `~/Arduino`.

Follow the directions at
[SAMD51 Thing Plus Hookup Guide](https://learn.sparkfun.com/tutorials/samd51-thing-plus-hookup-guide/setting-up-the-arduino-ide) to setup the Arduino IDE for
working with your SAMD51.

### Install LetheKit on your system

The LetheKit software can be placed anywhere you like.  It does not
need to be inside your Arduino sketchbook.  In these instructions
we'll presume you want to install it in `~/src/bc-lethekit`

Clone the git repository, being sure to enable `--recurse-submodules`:

    cd ~/src
    git clone --recurse-submodules \
        git@github.com:BlockchainCommons/bc-lethekit.git
    cd bc-lethekit
    export LK_ROOT=`pwd`
    
Install LetheKit into your Arduino sketchbook:

    # Adjust this, if neccessary for your own sketchbook location.
    export ARDUINO_ROOT=~/Arduino

    # Do the installation.
    $LK_ROOT/scripts/install-lethekit $LK_ROOT $ARDUINO_ROOT
    
The installation script creates symbolic links in
`$ARDUINO_ROOT/Libraries` which point back to the appropriate
dependencies in `$LK_ROOT/deps`.

### Notes

It's safe to re-run the installation at any time.

If you move your LetheKit source tree, just run the installation
script again from the new location (with the new value of `$LK_ROOT`).

If you want to uninstall LetheKit use the installation script with
the `--uninstall` option:

    # Uninstall LetheKit
    # FIXME - implement this!
    $LK_ROOT/scripts/install-lethekit --uninstall $LK_ROOT $ARDUINO_ROOT

It's safe to move your Arduino sketchbook at any time; LetheKit
should remain installed.

If you upgrade LetheKit in the future use the `--upgrade` option to
the install script after upgrading LetheKit:

    # After upgrading LetheKit
    # FIXME - implement this!
    $LK_ROOT/scripts/install-lethekit --upgrade $LK_ROOT $ARDUINO_ROOT
