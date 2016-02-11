#ifndef __SCANCODES_H__
#define __SCANCODES_H__

#define UTF8_MAX_LENGTH 4

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
	long int unicode; //the unicode number to send via alt+numpad or ^U if char is not available in a keyboard layout
};

struct keysym* toscan (const char* utf8);//returns the layout struct of a keysym
#endif
