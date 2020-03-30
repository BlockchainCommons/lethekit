## Lethe Kit Installation Instructions

Lethe Kit provides a collection of security modules (libraries) that
work well together.  You install them into your local Arduinio
development environment and then build provided sample applications or
develop your own applications.

These instructions describe how to install Lethe Kit into your local
development environment.  We presume you already have the Arduino IDE
installed and your "Sketchbook location" is `~/Arduino`.

### Install `arduino-cli`

Lethe Kit's installation script uses `arduino-cli` during
installation.

[Install `arduino-cli`](https://arduino.github.io/arduino-cli/installation/)

### Install Lethe Kit on your system

The Lethe Kit software can be placed anywhere you like.  It does not
need to be inside your Arduino sketchbook.  In these instructions
we'll presume you want to install it in `~/src/bc-lethe-kit`

Clone the git repository, make sure to enable `--recurse-submodules`:

    cd ~/src
    git clone --recurse-submodules \
        git@github.com:BlockchainCommons/bc-lethe-kit.git
    cd bc-lethe-kit
    export LK_ROOT=`pwd`
    
Install Lethe Kit into your Arduino sketchbook:

    # Adjust this, if neccessary for your own sketchbook location.
    export ARDUINO_ROOT=~/Arduino

    # Do the installation.
    $LK_ROOT/scripts/install-lethekit $LK_ROOT $ARDUINO_ROOT
    
The installation script creates symbolic links in
`$ARDUINO_ROOT/Libraries` which point back to the appropriate
dependencies in `$LK_ROOT/deps`.

### Notes

It's safe to re-run the installation at any time.

If you move your Lethe Kit source tree just run the installation
script again from the new location (with the new value of `$LK_ROOT`).

If you want to uninstall Lethe Kit use the installation script with
the `--uninstall` option:

    # Uninstall Lethe Kit
    # FIXME - implement this!
    $LK_ROOT/scripts/install-lethekit --uninstall $LK_ROOT $ARDUINO_ROOT

It's safe to move your Arduino sketchbook at any time, Lethe Kit
should remain installed.

If you upgrade Lethe Kit in the future use the `--upgrade` option to
the install script after upgrading Lethe Kit:

    # After upgrading Lethe Kit
    # FIXME - implement this!
    $LK_ROOT/scripts/install-lethekit --upgrade $LK_ROOT $ARDUINO_ROOT
