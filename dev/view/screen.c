#include <stdlib.h>
#include "screen.h"
struct screen *screen_start() {
	initscr();

	struct screen *screen = (struct screen*) malloc(sizeof(struct screen));
	screen->left = screen->centre = screen->right = NULL;
	return screen;
}

void screen_refresh(struct screen *screen) {
	refresh();

	if(screen->left != NULL)
		view_refresh(screen->left);
}

void screen_stop(struct screen *screen) {
	endwin();
}

struct view *create_view(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

	struct view *view = (struct view*) malloc(sizeof(struct view));
	view->x = x; 
	view->y = y; 
	view->w = w; 
	view->h = h;

	view->port = newwin(h,w,y,x);
	box(view->port, 0, 0);

	return view;
}

void view_refresh(struct view *view) {
	wrefresh(view->port);
}

