#ifndef SCREEN_H
#define SCREEN_H
#include <stdint.h>
#include <ncurses.h>
#include "views.h"
#include "state.h"


struct screen {
	struct view *left, *centre, *right;
};

struct screen *create_screen();

void screen_stop(struct screen*);

void screen_refresh(struct screen*);
void view_refresh(struct view*);
void init_ncurses();
#endif
