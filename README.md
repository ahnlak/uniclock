# UniClock

This project is a more fully rounded Clock for the Galactic Unicorn.

Features include:

- [x] fixed width font, to avoid the annoying shift in display
- [x] simple file-based WiFi configuration
- [x] NTP support
- [ ] comprehensive timezone handling
- [ ] automatic brightness adjustment for ambient light
- [ ] date display


## WiFi Configuration

To make things easier to commission, UniClock mounts as a drive when plugged
into your computer's USB port. This allows you to modify the configuration
file to define your WiFi connection details, timezone and other preferences.

Edit the `CONFIG.TXT` file with your local details.


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
