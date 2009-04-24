#!/bin/sh
exec objcopy -O binary -j .text "$1" "$2"
