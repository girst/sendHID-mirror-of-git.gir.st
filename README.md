# hardpass
A hardware password manager, built around a Paspberry Pi Zero and [`pass`, the UNIX password manager](https://passwordstore.org).

This is also an entry for [Hackaday.io](https://hackaday.io)'s Pi Zero Contest. 

## Hardware
I am using a Raspberry Pi Zero, since it has USB-OTG support and a lot of GPIO to interface an OLED screen, a button matrix for input and maybe even an ESP8266 for WiFi. 
The OLED I am intending to use has an I²C interface and a screen diagonal of .96". 

## using the driver
The neccessary drivers are available only in the Raspbian 4.4 Kernel, which you can install using
```
sudo BRANCH=next rpi-update
```
You also need to activate the device tree overlay `dwc2` and load the corresponding kernel module:
```
echo "dtoverlay=dwc2" | sudo tee -a /boot/config.txt
echo "dwc2" | sudo tee -a /etc/modules
```

To use this program, you have to enable the `libcomposite` driver on the Raspberry Pi and create a USB HID gadget. 
You can use this bash script:
```
#!/bin/bash
# this is a stripped down version of https://github.com/ckuethe/usbarmory/wiki/USB-Gadgets - I don't claim any rights

modprobe libcomposite
cd /sys/kernel/config/usb_gadget/
mkdir -p g1
cd g1
echo 0x1d6b > idVendor # Linux Foundation
echo 0x0104 > idProduct # Multifunction Composite Gadget
echo 0x0100 > bcdDevice # v1.0.0
echo 0x0200 > bcdUSB # USB2
mkdir -p strings/0x409
echo "fedcba9876543210" > strings/0x409/serialnumber
echo "girst" > strings/0x409/manufacturer 
echo "Hardpass" > strings/0x409/product
N="usb0"
mkdir -p functions/hid.$N
echo 1 > functions/hid.usb0/protocol
echo 1 > functions/hid.usb0/subclass
echo 8 > functions/hid.usb0/report_length
echo -ne \\x05\\x01\\x09\\x06\\xa1\\x01\\x05\\x07\\x19\\xe0\\x29\\xe7\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x81\\x02\\x95\\x01\\x75\\x08\\x81\\x03\\x95\\x05\\x75\\x01\\x05\\x08\\x19\\x01\\x29\\x05\\x91\\x02\\x95\\x01\\x75\\x03\\x91\\x03\\x95\\x06\\x75\\x08\\x15\\x00\\x25\\x65\\x05\\x07\\x19\\x00\\x29\\x65\\x81\\x00\\xc0 > functions/hid.usb0/report_desc
C=1
mkdir -p configs/c.$C/strings/0x409
echo "Config $C: ECM network" > configs/c.$C/strings/0x409/configuration 
echo 250 > configs/c.$C/MaxPower 
ln -s functions/hid.$N configs/c.$C/
ls /sys/class/udc > UDC
```

## modifying `pass`
The version of (pass)[https://passwordstore.org] doesn't format the output as nice, so it is easier to work with. Using the `pass-installer.sh` will automatically clone from the original repository, apply the patch and install it (make sure, `patch` and `sudo` is installed). 

You can then check out `hardpass-demo.sh` to see how to supply a passphrase directly from the command line (without gpg-agent). Keep in mind, that it will show up in your history file!
