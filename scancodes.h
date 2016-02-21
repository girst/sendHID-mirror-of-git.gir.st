// (C) 2016 Tobias Girstmair, released under the GNU GPL
#ifndef __SCANCODES_H__
#define __SCANCODES_H__

#define UTF8_MAX_LENGTH 4

#define MOD_NONE   0
#define MOD_LCTRL  1<<0
#define MOD_LSHIFT 1<<1
#define MOD_LALT   1<<2
#define MOD_LSUPER 1<<3
#define MOD_RCTRL  1<<4
#define MOD_RSHIFT 1<<5
#define MOD_RALT   1<<6
#define MOD_RSUPER 1<<7

struct layout {
	unsigned short key; //scancode of normal key
		//if this is NULL, the key does not exist in this layout.
	unsigned short mod; //bitmask of modifier keys
	short is_dead; //is dead key (needs to be pressed twice)
};
struct keysym {
	char sym [UTF8_MAX_LENGTH]; //utf-8 encoded key symbol
	struct layout en_us; //substructure for this layout
	struct layout de_at;
	struct layout de_nd;
	unsigned int unicode; //the unicode number to send via alt+numpad or ^U if char is not available in a keyboard layout
};

enum kbdl {  //keyboard layouts:
        na_NA, //reserved
        en_US,
        de_AT,
        de_ND //de_AT-nodeadkeys
};

struct keysym* toscan (const char* utf8);//returns the layout struct of a keysym
struct layout* tolay (struct keysym* s, enum kbdl layout); //returns layout struct from keysym struct
#endif
