#include <stdlib.h>
#include "screen.h"
struct screen *screen_start() {
	initscr();

	return (struct screen*) malloc(sizeof(struct screen));
}

void screen_refresh(struct screen *screen) {
	printw("World, Greetings!");
	refresh();
}

void screen_stop(struct screen *screen) {
	endwin();
}
