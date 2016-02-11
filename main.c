/*
description: sends a sequence of keystrokes to the hid device. 
parameters:
	device file (e.g. /dev/hidg0)
	keyboard layout (1=en_us, 2=de_at, 3=de_at-nodeadkeys)
	unicode method: 1=gtk_holddown, 2=gtk_spaceend, 3=windows
	the string to send (as whitespace is important, the `echo` way of concatenating all parameters is not supported. if your string has white space in it and you are in an interactive session, quote your string.)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scancodes.h"

//argv-indices
#define P_EXE 0 //executable name
#define P_DEV 1 //device file
#define P_LAY 2 //layout
#define P_UNI 3//unicode method
#define P_STR 4 //string to type
#define NUM_P P_STR+1 //number of parameters

void send_key (FILE* hid_dev, unsigned short key, unsigned short mod);

int main (int argc, char** argv) {
	if (argc != NUM_P) {
		fprintf (stderr, "Usage: %s <device file> <layout> <unicode> \"<string>\"\n", argv[P_EXE]);
		return 1;
	}
	FILE* hid_dev = fopen ("/dev/hidg0", "w");
	for (int i = 0; i < strlen (argv[P_STR]); i++) {

		char tmp[UTF8_MAX_LENGTH] = {argv[P_STR][i], argv[P_STR][i+1], argv[P_STR][i+2], '\0'};
		//TODO: replace by something less stupid
		if (argv[P_STR][i] < 128) { // not multi byte
			tmp[1] = '\0';
		} else { // is multi byte
			if (argv[P_STR][i] < 0xe0) {
			i++; //skip next thing
			tmp[2] = 0;
			} else {
			i+=2; //WARN: fails on utf8 > 3 byte
			}
		}

		struct keysym* s = toscan (tmp);
		if (s == NULL) {
			fprintf (stderr, "Key Symbol not found.\n");
			return 1;
		}
		struct layout* l;
		int ignore_deadkey = 0;
		switch (atoi (argv[P_LAY])) {
		case 0:
			fprintf (stderr, "This keyboard layout is reserved.\n");
			return 1;
		case 1: //en_us
			l = &(s->en_us);
			break;
		case 2: //de_at
			l = &(s->de_at);
			break;
		case 3: //de_at-nodeadkeys
			l = &(s->de_at);
			ignore_deadkey = 1;
			break;
		default:
			fprintf (stderr, "Unrecognised keyboard layout.\n");
			return 1;
		}
		if (l->key == 0x00) {
			//key does not exist in this layout
			fprintf (stderr, "Key not in this layout!\n");
			/*TODO: send unicode sequence
			  there are different methods to be used for gtk and
			  winblows. ctrl-shift-u-HEX vs. ctrl-shift-u,HEX,SPACE
			  vs. alt+NUMPAD vs. alt+'+'+HEX
			*/
			switch (atoi (argv[P_UNI])) {
			case 0: //skip unicode character entry
				break;
			case 1: //gtk: hold ctrl and shift while entering
				//TODO
				break;
			case 2: //gtk: use space as end marker for unicode
				//TODO
			case 3: //windows: alt+numpad (decimal)
				//TODO
				break;
			default:
				fprintf (stderr, "Unicode Method unknown!\n");
				return 1;
			}
		} else {
			send_key(hid_dev, l->key, l->mod);
			send_key(hid_dev, '\0', '\0'); //release all keys
			if (l->is_dead && !ignore_deadkey) {
				//dead keys need to be pressed twice to show up
				send_key(hid_dev, l->key, l->mod);
				send_key(hid_dev, '\0', '\0'); //release all keys
			}
		}
	}
	fclose (hid_dev);

	return 0;
}

void send_key (FILE* hid_dev, unsigned short key, unsigned short mod) {
	fprintf (hid_dev, "%c%c%c%c%c%c%c%c", mod, '\0', key, '\0', '\0', '\0', '\0', '\0');
}
