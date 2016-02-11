all:
	gcc -std=c99 -Wall -Werror main.c scancodes.c -o scan
