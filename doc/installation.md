## LetheKit Installation Instructions

These instructions describe how to install LetheKit into your local
development environment.

LetheKit provides a collection of security modules (libraries) that
work well together. You install them into your local Arduino
development environment and then build the provided sample applications or
develop your own applications before uploading them to your SAMD51.


### Installing Arduino IDE

If you don't already have the *Arduino IDE* installed (recommended v1.8.13) in your local development environment, here are the instructions:

- For macOS we recommend using `$ brew cask install arduino` which will install the Arduino.app in your `/Applications/` folder.
- For Linux users we recommend installing with:
```bash
$ wget https://downloads.arduino.cc/arduino-1.8.13-linux64.tar.xz
$ tar -xvf arduino-1.8.13-linux64.tar.xz
$ cd arduino-1.8.13
$ ./install.sh # sudo ./install.sh
```

### Installing Toolchain for SAMD51

This is an excerpt from [SAMD51 Thing Plus Hookup Guide](https://learn.sparkfun.com/tutorials/samd51-thing-plus-hookup-guide/setting-up-the-arduino-ide):

- In the Arduino IDE, open **Preferences** (**Arduino** > **Preferences**), find the  **Additional Board Manager URLs** text box, and copy this  URL to it and click OK.
  -  `https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json`
- Now we need to install the correct Arduino SAMD Core. Navigate to your **Board Manager** (**Tools** > **Board**: > **Boards Manager...**), then find an entry for **Arduino SAMD Boards (32-bits ARM Cortex-M0+)**. It may help to enter `SAMD` into the search bar. Select that Arduino core and install the latest version.
- Now we need to install the **SparkFun SAMD** Add-on. Enter `sparkfun samd` in the **Board Manager**'s search bar, which should show **SparkFun SAMD Boards**. Install it.
- Now select from the **Tools** > **Board**: > list, choose **SparkFun SAMD51 Thing Plus**. Now when you look at the **Tools** menu you should see this board selected as default.
- Connect the LetheKit to your computer and turn it on, and then navigate back up to the **Tool** > **Port** menu. The port menu should magically know which of your ports is the SAMD51 Thing Plus board.

### Installing LetheKit on your system

Clone the git repository anywhere you like and install LetheKit:

```bash
$ git clone --recursive https://github.com/BlockchainCommons/bc-lethekit
$ cd bc-lethekit
$ export LK_ROOT=`pwd` && $LK_ROOT/scripts/install-lethekit $LK_ROOT
```

The installation script creates symbolic links in
`Arduino/Libraries` which point back to the appropriate
dependencies in `$LK_ROOT/deps`.

#### Notes

It's safe to re-run the installation at any time.

If you move your LetheKit source tree or pull new changes in from Github, just run the installation
script again:

```bash
$ cd bc-lethekit
$ export LK_ROOT=`pwd` && $LK_ROOT/scripts/install-lethekit $LK_ROOT
```

### Next steps

Proceed to [Seedtool Installation Instructions](../seedtool/doc/build.md) to compile and install the `seedtool` firmware on your LetheKit.

<!-- TODO
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
-->
