# hardpass
A hardware password manager, built around a Raspberry Pi Zero and [`pass`, the UNIX password manager](https://passwordstore.org).

This project is now maintained at [this repository](https://github.com/girst/hardpass-passwordmanager). 

# sendHID

sendHID is a tool to simulate typing using the Linux USB Gadget mode. 

Use a Raspberry Pi Zero (or A) to send keystrokes to a host computer. 

## using the driver
There are two drivers available: the legacy `g_hid` driver, which has windows support, and the new `libcomposite`, which makes emulation of multiple devices at the same time very easy. Setup instructions on the latter are below. 

The neccessary drivers are available only in the Raspbian 4.4 Kernel, which you can install using `sudo BRANCH=next rpi-update`, if you haven't updated you Pi in a long time. 

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

## internals of this code
whenever you need to add (or remove) a new keyboard layout, the following changes have to be made:

### adding a new key to existing layouts

1. if the key is on the keyboard, just add a new line, and use the keycode ("Usage ID (Hex)") from the table 12 on page 53pp of the [_USB HID Usage Tables_](https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf) document. 

Following the keycode comes the modifier bit mask. `0x02` is Shift, `0x40` AltGraph. 

Now an example adding the letter A:

```
{"A",  //character to translate
	{ //english layout
		0x04, //keycode
		0x02  //shift
	}, { //german layout
		0x04, //keycode
		0x02  //shift
	}, { //german-nodeadkeys
		0x04, //keycode
		0x02  //shift
	}
}
```

2. if it isn't on the keyboard, fill keyocde and modifier bit mask with `0x00` for the layout and use the `.unicode=0xXXXX` (See point 3 in the 'adding a new layout' section below). 

### adding a new keyboard layout (example)
1. in `scancodes.h` add the layout to `keysym`-struct:
this identifier does not need to be used outside of scancodes.{h,c}. 
```
struct keysym {
	// ...
	struct layout de_dv; //dvorak layout
	// ...
};
```
2. the enum `kbd1` has to be ammended:
```
enum kbdl {  //keyboard layouts:
	// ...
        de_DV //de_AT-Dvorak
};
```
3. in `scancodes.c` you need to add a new column (containing more columns) to the big `keysyms[]` table:
It is suggested to explicitly name `.is_dead` and `.unicode` to avoid confusion. Also notice that `.is_dead` is part of the layout (and goes within the inner braces), while `.unicode` resides in the keysym-struct. 
if you want to add new keys, the following must be kept in mind: `toscan()` will use the nth line of the table, if n is larger than 32 (aka. will use the ascii code to look up chars). symbols not in the 7-bit ascii standard can be put on the first 32 positions as utf-8 encoded strings, although keeping [0] to the release all chars is recommended. (some text editors will convert files to older encodings - this might break things)
```
struct keysym keysyms[] = {
	//...
	{"#", {0x20, 0x02}, {0x31, 0x00}, {0x31, 0x00}, {KEY, MODIFIER}},
	{"^", {0x23, 0x02}, {0x35, 0x00}, {0x35, 0x00}, {KEY, MODIFIER, .is_dead = 1}},
	{"&", {0x24, 0x02}, {0x23, 0x02}, {0x23, 0x02}, {KEY, MODIFIER}, .unicode=OxHEX},
	//...
};
```
4. `tolay()` needs a new case to its switch statement:
This is the only place where the `layout`-idenitfier from `keysym` needs to be used. 
```
struct layout* tolay (struct keysym* s, enum kbdl layout) {
        switch (layout) {
	// ...
        case de_DV: return &(s->de_dv);
        default: return NULL;
        }
}
```
5. finally, your code must understand the new layout, represented by the `enum kbd1`-entry, `de_DV` in this example. 
