

## Installing *Seedtool* on *LetheKit*

### Installing Arduino IDE

If you don't already have the *Arduino IDE* installed in your local development environment, here are the instructions:

- You will need to install the latest version of the *Arduino IDE*.
  - For Linux ??
  - For macOS as of the creation of this document _v1.8.12_.
- Download the zip file from the *Arduino IDE* [download page](http://arduino.cc/en/Main/Software) and move the Arduino app to the appropriate folder.

  - For macOS we recommend that you instead use `brew cask install arduino` which will install the Arduino.app in your `/Applications/` folder.
- Open the *Aurduino IDE* application one time:
  - On Linux

  ```
  open Arduino
  ```

  - On Mac

  ```
  open -a Arduino
  ```
 This let the *Arduino IDE* create the default folder and sketchbook.
  - On Linux this will be in  `~/Arduino/`
  - In macOS this will be in `~/Documents/Arduino`

### Preparing the *Arduino IDE* for building *LetheKit*

You'll need to add files to the *Arduino IDE* to support the [SAMD51 Thing Plus](https://www.sparkfun.com/products/14713) board used by *LetheKit*.

- In the Arduino app, open **Preferences** (**Arduino** > **Preferences**), find the  **Additional Board Manager URLs** text box, and copy this  URL to it and click OK.
  -  `https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json1`
- Now we need to install the correct Arduino SAMD Core. Navigate to your **Board Manager** (**Tools** > **Board**: > **Boards Manager...**), then find an entry for **Arduino SAMD Boards (32-bits ARM Cortex-M0+)**. It may help to enter `SAMD` into the search bar. Select that Arduino core and install the latest version (as of the creation of this document _v1.8.6_).
- Now we need to install the **SparkFun SAMD** Add-on. Enter `sparkfun samd` in the **Board Manager**'s search bar, which should show **SparkFun SAMD Boards** with a dependency to the SAMD Core you just installed. Install this (as of the creation of this document *v1.7.4*).
- Now select from the **Tools** > **Board**: > list, choose **SparkFun Thing Plus**. Now when you look at the **Tools** menu you should see this board selected as default.
- Now you need to select the serial port. Connect the LetheKit to your computer and turn it on, and then navigate back up to the **Tool** > **Port** menu. The port menu should magically know which of your ports (if you have more than one) is the SAMD51 Thing Plus board.
  - On Linux, the port should look something like ??
  - In macOS the port should look something like "**/dev/cu.usbmodem####**", not a bluetooth port.
- Next we need to install the various hardware libraries for *LetheKit*. Navigate to your Library Manager (**Tools** > **Manage Libraries…**).

  - Display Library: Enter `GxEPD2` in the Library Manager's search bar, which should show **GxEPD2** library. Install this (as of the creation of this document *v1.2.6*), and all dependencies.
  - Keypad: Enter `Keypad` in the Library Manager's search bar, which should show a number of libraries, but only one is called just **Keypad** library. Install this (as of the creation of this document *v3.1.1*).

### Preparing LetheKit Libraries for installation

*LetheKit* provides a collection of security submodules (libraries) that work well together. You install them into your local *Arduino* development environment and use them to build the provided sample *Seedtool* application or develop your own applications before uploading them to your SAMD51 board used by *LetheKit*

The LetheKit software can be placed anywhere you like. It does not need to be inside your Arduino sketchbook. These instructions we'll presume you want to install it in `src/bc-lethekit` inside your default Arduino directory.

- Using Terminal, navigate to your default Arduino folder (on Linux in our ~ home folder, on macOS typically in `/Documents`), and export this location:

  ```
  # Linux
  cd ~/Arduino
  # macOS
  # cd ~/Documents/Arduino
  export ARDUINO_ROOT=`pwd`
  echo $ARDUINO_ROOT
  ```
  You should see output of the correct full path.

- Now if you've not done it before, from the Terminal create a `src` folder and go there:

  ```
  mkdir src
  cd src
  ```

- Now clone the *LetheKit*, export LK_ROOT, and run `scripts/install-lethekit`.

  ```
  git clone --recurse-submodules \
      git@github.com:BlockchainCommons/bc-lethekit.git
  cd bc-lethekit
  export LK_ROOT=`pwd`
  echo $LK_ROOT
  ```
  You should now see the correct full path.

* Next we need to to run a script to create symbolic links to connect all the LetheKit submodule libraries together.

  ```
  $LK_ROOT/scripts/install-lethekit $LK_ROOT $ARDUINO_ROOT
  ```

### Compile and Install *Seedtool*

In the `seedtool/` directory we should install `gitrevision.h`. The `gitrevision.h` file is generated with git hooks and contains the latest version information, allowing that information to be built into the application and reported to the user. It is not critical, but is very helpful when reporting bugs, requesting features etc.

Enable the generation of the `gitrevision.h` file. This only needs to be done once, but is safe to do anytime you like.

```
# Must be run in the seedtool directory
cd seedtool
./enable-gitrevision-hooks.sh
```

You may disable `gitrevision.h` generation with:

```
# Must be run in the seedtool directory
./disable-gitrevision-hooks.sh
```

Open the seedtool/seedtool.ino sketchbook.

- On Linux

```
open Arduino ./seedtool.intro
```

- On Mac

```
open -a Arduino ./seedtool.ino
```

Now we will build *Seedtool* and install it.

- Connect a *LetheKit* hardware device to an appropriate USB port and turn it on.

To upload code to the SAMD51 module you will need to manually enter bootloader mode.

- There are two approaches, the first involves opening the case and pressing the reset button twice in rapid succession.

- It is also possible to enter bootloader mode by power cycling with the external power switch. Here are the steps for the external power switch method:

  1. Connect the LetheKit to a USB port on you computer. Turn the LetheKit on and wait a few seconds.
  2. Turn off, on, off, on rapidly.
  3. If the device is in bootloader mode, the blue power light will not be on, even though it is plugged in and the switch is left in the "on" position.
  
  A description of using the internal reset button can be found at the [SAMD51 Thing Plus Hookup Guide](https://learn.sparkfun.com/tutorials/samd51-thing-plus-hookup-guide/setting-up-the-arduino-ide), search for "Don't Forget to Launch the Bootloader".
  
- Compile list & Press the "Upload" button on the Arduino IDE. The *Seedtool* application will be built and uploaded to your LetheKit device, and it will be autostarted.

- You will now see some testing output to the LetheKit display, and then see the *Seedtool*'s initial screen displaying the current version number of *Seedtool*.

- *Seedtool* on your *LetheKit* is ready for use!

See [Seedtool Application Instructions](https://github.com/BlockchainCommons/bc-lethekit/blob/master/seedtool/README.md) for more information on how Seedtool works.