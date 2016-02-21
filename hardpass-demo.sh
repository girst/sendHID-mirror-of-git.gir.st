#!/bin/bash
# (C) 2016 Tobias Girstmair, released under the GNU GPL

# this is a demo that fetches a password from the password store (must be initialized) and types it over the usb-hid interface to the host computer. 
# see github.com/girst/hardpass -> readme for how to initialize the driver. 
# since 471f0ed text to type is read from stdin instead of the last parameter!

PASSWORD_STORE_GPG_OPTS="--passphrase 123456789" pass show github.com/girst | head -n 1 | sudo ./scan /dev/hidg0 2 2
#                                     `-------´            `--------------´   `-------´        `----´ `--------´ ^ ^
#                                      '>demo password      '>pass-entry       | get only       |      |         | '>unicode method
#                                                                              '>first line     |      |         '>keyboard layout
#                                                                                               |      '>device file of hid-gadget
#                                                                                               '>name of executable
