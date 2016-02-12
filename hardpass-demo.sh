#!/bin/bash

# this is a demo that fetches a password from the password store (must be initialized) and types it over the usb-hid interface to the host computer. 
# see github.com/girst/hardpass -> readme for how to initialize the driver. 

 sudo ./scan /dev/hidg0 2 2 $(PASS_GPG_PHRASE=123456789 pass show github.com/girst|head -n 1)
#`--´ `----´ `--------´ ^ ^                   `-------´           `--------------´ `-------´
#  |     |       |      | |                       |                       |            |
#  |     |       |      | |                       |                       |            '>make sure to only fetch the first line (containing the password)
#  |     |       |      | |'>unicode method       '>demo password         '>passwordstore-entry
#  |     |       |      '>keyboard layout
#  |     |       '>device file created by the libcomposite driver
#  |     '>name of the executable i wrote
#  '>device file access needs root permissions (or chmodding)

