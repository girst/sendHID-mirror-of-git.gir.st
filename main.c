/*
description: sends a sequence of keystrokes to the hid device. 
parameters:
	device file (e.g. /dev/hidg0)
	keyboard layout (e.g. en_us; only a limited number are supported and they do not follow unix-naming conventions for convenience)
	the string to send (as whitespace is important, the `echo` way of concatenating all parameters is not supported. if your string has white space in it and you are in an interactive session, quote your string. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scancodes.h"

void send_key (FILE* hid_dev, unsigned short key, unsigned short mod);

int main (int argc, char** argv) {
	if (argc != 4) {
		fprintf (stderr, "Usage: %s <device file> <layout> \"<string>\"\n", argv[0]);
		return 1;
	}
	FILE* hid_dev = fopen ("/dev/hidg0", "w");
	for (int i = 0; i < strlen (argv[3]); i++) {

		char tmp[UTF8_MAX_LENGTH] = {argv[3][i], argv[3][i+1], argv[3][i+2], '\0'};
		//TODO: replace by something less stupid
		if (argv[3][i] < 128) { // not multi byte
			tmp[1] = '\0';
		} else { // is multi byte
			if (argv[3][i] < 0xe0) {
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
		switch (atoi (argv[2])) {
		case 0:
			fprintf (stderr, "This keyboard layout is reserved.\n");
			return 1;
		case 1: //en_us
			send_key(hid_dev, s->en_us.key, s->en_us.mod);
			break;
		case 2: //de_at
			send_key(hid_dev, s->de_at.key, s->de_at.mod);//, s->de_at.is_dead);
			break;
		case 3: //de_at-nodeadkeys
			send_key(hid_dev, s->de_at.key, s->de_at.mod);
			break;
		default:
			fprintf (stderr, "Unrecognised keyboard layout.\n");
			return 1;
		}
		send_key(hid_dev, '\0', '\0');
	}
	fclose (hid_dev);

	return 0;
}

void send_key (FILE* hid_dev, unsigned short key, unsigned short mod) {
	fprintf (hid_dev, "%c%c%c%c%c%c%c%c", mod, '\0', key, '\0', '\0', '\0', '\0', '\0');
}
