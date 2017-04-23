/*
(C) 2016 Tobias Girstmair, released under the GNU GPL

description: sends a sequence of keystrokes provided from stdin to the hid
device. 
stops typing at: control characters (including newline), chars not in table, EOF
parameters:
	device file (e.g. /dev/hidg0)
	keyboard layout (1=en_us, 2=de_at, 3=de_at-nodeadkeys)
	unicode method: 1=gtk_holddown, 2=gtk_spaceend, 3=windows
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scancodes.h"

#define TEXT_LEN 256 //max length of piped text
enum params {//argv-indices:
	P_EXE, //executable name
	P_DEV, //device file
	P_LAY, //layout
	P_UNI, //unicode method
	NUM_P  //number of parameters
};
enum uni_m {//unicode methods:
	SKIP, //ignore any keys not on the layout
	GTK_HOLD, //hold ctrl and shift while entering hex values
	GTK_SPACE, //end hex sequence with spacebar
	WINDOWS //use alt+numpad
};
enum errors {
	ERR_SUCCESS, //no error
	ERR_ARGCOUNT, //wrong number of arguments
	ERR_SYMBOL, //symbol not in look up table
	ERR_LAYOUT, //parameter P_LAY does not contain a correct keyboard layout
	ERR_LAZY //i haven't done this
};

void send_key (FILE* hid_dev, unsigned short key, unsigned short mod);
enum errors send_unicode (FILE* hid_dev, unsigned int unicode, enum uni_m method, enum kbdl layout);

int main (int argc, char** argv) {
	if (argc != NUM_P) {
		fprintf (stderr, "Usage: %s <device file> <layout> <unicode>\n", argv[P_EXE]);
		fprintf (stderr, "Takes string to type from stdin\n");
		fprintf (stderr, "<device file>:\ton the Raspberry Pi usually /dev/hidg0\n");
		fprintf (stderr, "<layout>:\n\t%d\t%s\n\t%d\t%s\n\t%d\t%s\n", 
			en_US, "en_US", 
			de_AT, "de_AT (w/ dead keys)", 
			de_ND, "de_AT-nodeadkeys");
		fprintf (stderr, "<unicode>:\n\t%d\t%s\n\t%d\t%s\n\t%d\t%s\n\t%d\t%s\n", 
			SKIP, "skip over unicode characters", 
			GTK_HOLD, "X11 Holddown: CTRL+SHIFT+[u, hex]",
			GTK_SPACE, "X11 Space: CTRL+SHIFT+u, hex, SPACE",
			WINDOWS, "Windows: Alt+[Numpad]");
		return ERR_ARGCOUNT;
	}
	FILE* hid_dev = fopen (argv[P_DEV], "w");
	char in_string[TEXT_LEN];
	fgets(in_string, TEXT_LEN, stdin);
	for (int i = 0; i < strlen (in_string); i++) {
		if (in_string[i] < 32) {
			if (in_string[i] != '\n' && in_string[i] != '\t') {
				fprintf (stderr, "Cannot print control characters!\n");
				return ERR_SYMBOL;
			}
		}

		char tmp[UTF8_MAX_LENGTH] = {in_string[i], in_string[i+1], in_string[i+2], '\0'};
		//TODO: replace by something less stupid
		if (in_string[i] < 128) { // not multi byte
			tmp[1] = '\0';
		} else { // is multi byte
			if (in_string[i] < 0xe0) {
			i++; //skip next thing
			tmp[2] = 0;
			} else {
			i+=2; //WARN: fails on utf8 > 3 byte
			}
		}

		struct keysym* s = toscan (tmp);
		if (s == NULL) {
			fprintf (stderr, "Key Symbol not found.\n");
fclose (hid_dev);
			return ERR_SYMBOL;
		}
		struct layout* l = tolay (s, atoi (argv[P_LAY]));
		if (l == NULL) {
			fprintf (stderr, "Unrecognised keyboard layout.\n");
fclose (hid_dev);
			return ERR_LAYOUT;
		}
		if (l->key != 0x00) {
			send_key(hid_dev, l->key, l->mod);
			send_key(hid_dev, '\0', '\0'); //release all keys
			if (l->is_dead) {
				//dead keys need to be pressed twice to show up
				send_key(hid_dev, l->key, l->mod);
				send_key(hid_dev, '\0', '\0'); //release all keys
			}
		} else {
			//key does not exist in this layout, use unicode method
			//fprintf (stderr, "Warning: Key '%s'(0x%x) not in this layout!\n", s->sym, s->unicode);
			send_unicode (hid_dev, s->unicode, atoi (argv[P_UNI]), atoi(argv[P_LAY]));
		}
	}
	fclose (hid_dev);

	return ERR_SUCCESS;
}

void send_key (FILE* hid_dev, unsigned short key, unsigned short mod) {
	fprintf (hid_dev, "%c%c%c%c%c%c%c%c", mod, '\0', key, '\0', '\0', '\0', '\0', '\0');
}

enum errors send_unicode (FILE* hid_dev, unsigned int unicode, enum uni_m method, enum kbdl layout) {
	char buf[10];
	struct keysym* s;
	struct layout* l;

	if (unicode == 0x00) {
		fprintf (stderr, "Symbol not in lookup table!\n");
		return ERR_SYMBOL;
	}

	switch (method) {
	case SKIP:
		break;
	case GTK_HOLD:
		sprintf (buf, "%x", unicode);
		s = toscan ("u");
		l = tolay (s, layout);
		send_key (hid_dev, l->key, MOD_LCTRL | MOD_LSHIFT);
		for (int i = 0; i < strlen (buf); i++) {
			s = toscan ((char[2]){buf[i], '\0'});
			l = tolay (s, layout);
			send_key (hid_dev, l->key, MOD_LCTRL | MOD_LSHIFT);
		}
		send_key (hid_dev, '\0', '\0');
		break;
	case GTK_SPACE:
		sprintf (buf, "%x ", unicode);
		s = toscan ("u");
		l = tolay (s, layout);
		send_key (hid_dev, l->key, MOD_LCTRL | MOD_LSHIFT);
		for (int i = 0; i < strlen (buf); i++) {
			s = toscan ((char[2]){buf[i], '\0'});
			l = tolay (s, layout);
			send_key (hid_dev, l->key, MOD_NONE);
		}
		send_key (hid_dev, '\0', '\0');
		break;
	case WINDOWS:
		fprintf (stderr, "windows method not implemented!\n");
		return ERR_LAZY;
	default:
		fprintf (stderr, "unknown unicode method!\n");
		return ERR_LAYOUT; //TODO: better error code
	}
	return ERR_SUCCESS;
}
