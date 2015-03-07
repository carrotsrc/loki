#include <stdlib.h>
#include "screen.h"
#include "capture.h"
struct screen *create_screen() {

	struct screen *screen = (struct screen*) malloc(sizeof(struct screen));
	screen->left = screen->centre = screen->right = NULL;
	return screen;
}

void write_screen(struct screen *screen, struct loki_state *state) {
	pthread_mutex_lock(&scrmutex);
	if(screen->left != NULL) {
		screen->left->write(state, screen->left->port);
	}

	if(screen->centre != NULL) {
		werase(screen->centre->port);
		screen->centre->write(state, screen->centre->port);
	}

	if(screen->right != NULL) {
		werase(screen->right->port);
		screen->right->write(state, screen->right->port);
	}

	move(0,15);
	printw("Total Packets: %ld | ", ((struct frame_log*)(state->log))->totalPackets);

	screen_refresh(screen);
	pthread_mutex_unlock(&scrmutex);
}

void screen_refresh(struct screen *screen) {
	refresh();

	if(screen->left != NULL)
		view_refresh(screen->left);

	if(screen->centre != NULL)
		view_refresh(screen->centre);

	if(screen->right != NULL)
		view_refresh(screen->right);

	move(LINES-1, 1);
	doupdate();
}

void screen_stop(struct screen *screen) {
	endwin();
}


void init_ncurses() {
	initscr();
	start_color();
	use_default_colors();
	noecho();

	init_pair(1, COLOR_GREEN, -1);
}

