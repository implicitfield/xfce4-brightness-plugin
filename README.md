# xfce4-brightness-plugin

This is a modernized version of the old brightness plugin that shipped in `xfce4-power-manager` prior to version 1.3.1.

## How does it work?

This plugin directly communicates with the kernel via files in `/sys/class/backlight/<vendor>`. As such, your user will need to be a member of the appropriate group (usually `video`) to allow writing to `/sys/class/backlight/<vendor>/brightness`.

## Screenshot

![Screenshot](screenshot.png)

## Building

Run `build.sh`.

## Installation

Run `install.sh` as root.
