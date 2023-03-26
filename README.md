# UniClock

This project is a more fully rounded Clock for the Galactic Unicorn.

Features include:

- [x] fixed width font, to avoid the annoying shift in display
- [x] simple file-based WiFi configuration
- [x] NTP support
- [ ] comprehensive timezone handling
- [x] automatic brightness adjustment for ambient light
- [x] date display
- [ ] alternative display options

## Operation

I've kept a lot of the controls in line with Pimoroni's example `clock.py`.

The brightness of the display is adjusted with the 'LUX +/-' buttons on the right
hand side of the Unicorn; this brightness is modified by the ambient light levels,
to turn the display brightness down when it's darker. This should mean the display
is more readable in a bright room, and less blinding in a darkened one.

The offset from UTC is adjusted with the 'VOL +/-' buttons on the right hand side;
this allows you to tweak the offset by whole hours only (but if you live in an
odd partial-hour timezone you can edit `CONFIG.TXT` - see below), will tell you
the current setting and stop you going too far. This setting will be saved you you.

The 'D' button on the left hand side will briefly display the current date.


## WiFi Configuration

To make things easier to commission, UniClock mounts as a drive when plugged
into your computer's USB port. This allows you to modify the configuration
file to define your WiFi connection details, timezone and other preferences.

Edit the `CONFIG.TXT` file with your local details. Note that the formatting is
a little fussy (a space is required after the colon; just follow the default
settings).

|Entry|Default||
|---|---|---|
|`SSID`|unknown|The SSID of your WiFi network|
|`PASSWORD`|unknown|The password of your WiFi network|
|`NTP_SERVER`|pool.ntp.org|The NTP server to query|
|`UTC_OFFSET`|60|The amount of minutes to add to UTC to get your local time|
|`DATE_FORMAT`|dmy|`dmy` = dd/mm/yyyy, `mdy` = mm/dd/yyyy|


## Building

If you wish to build from source rather than grabbing the latest release, the
usual build commands apply. These instructions are for Linux or Linux-like
environments; things may (will?) be different under Windows.


### Basic build requirements

Install build requirements:

```bash
sudo apt update
sudo apt install cmake gcc-arm-none-eabi build-essential
```

And the Pico SDK:

```
git clone https://github.com/raspberrypi/pico-sdk
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=`pwd`
cd ../
```

The `PICO_SDK_PATH` set above will only last the duration of your session.

You should should ensure your `PICO_SDK_PATH` environment variable is set by `~/.profile`:

```
export PICO_SDK_PATH="/path/to/pico-sdk"
```


### Fetch both this repository, and the Pimoroni Pico libraries

```
git clone https://github.com/pimoroni/pimoroni-pico
git clone https://github.com/ahnlak/uniclock
cd uniclock
```


### Compile it

```
mkdir build
cd build
cmake ..
make
```
