#ifndef SCREEN_H
#define SCREEN_H
#include <stdint.h>
#include <ncurses.h>

struct view {
	uint16_t x, y, w, h;
	WINDOW *port;
};

struct screen {
	struct view *left, *centre, *right;
};

struct screen *create_screen();
struct view *create_view(uint16_t,uint16_t,uint16_t,uint16_t);

void screen_stop(struct screen*);

void screen_refresh(struct screen*);
void view_refresh(struct view*);
void init_ncurses();
#endif
