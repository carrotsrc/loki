#ifndef SCREEN_H
#define SCREEN_H
#include <stdint.h>
#include <ncurses.h>

struct view {
	uint16_t x, y, width, height;
};

struct screen {
	struct view *left, *centre, *right;
};

struct screen *screen_start();
void screen_stop(struct screen*);
void screen_refresh(struct screen*);
#endif
