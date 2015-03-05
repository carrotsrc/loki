#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/if_ether.h>
#include <ncurses.h>

#include "state.h"
#include "monitor.h"
#include "capture.h"
#include "view/screen.h"

/* TODO:
 * this needs to be researched more deeply
 * to see what net-tools is up to...
 */
static int ifconfig_device_up(const char *dev, const char *address) {


	char ifcfg[40];
	if(address == NULL)
		sprintf(ifcfg, "ifconfig %s up", dev);
	else
		sprintf(ifcfg, "ifconfig %s %s up", dev, address);

	printf("$ %s\n", ifcfg);

	return system(ifcfg);
}

static int ifconfig_device_down(const char *dev) {
	char ifcfg[20];
	sprintf(ifcfg, "ifconfig %s down", dev);
	printf("$ %s\n", ifcfg);

	return system(ifcfg);
}

static void create_screens(struct loki_state*);
static void input_loop(struct loki_state*);
int main( int argc, char *argv[]) {

	pthread_t tcap, tui;
	struct loki_state lstate;
	char *addr, *dev;
	int r;

	cbreak();

	lstate.mode = OVERVIEW;

	addr = "192.168.0.1";
	if(argc < 2) {
		fprintf(stderr, "Device unspecified\n");
		exit(EXIT_FAILURE);
	}

	if(argc >= 3) {
		addr = argv[2];
	}

	dev = argv[1];
	r = 0;

	/* switch on monitor mode using libnl */
	if(set_monitor_mode(dev) > 0) {
		fprintf(stderr, "Error on setting monitor flag\n");
		exit(EXIT_FAILURE);
	}
	/* If we're here then we should be in monitor mode.
	 */

	lstate.dev = dev;

	r = ifconfig_device_up(dev, addr);
	if(r != EXIT_SUCCESS) {

		if( r == -1)
			fprintf(stderr, "Error in creating child process\n");
		else if(r == EXIT_FAILURE)
			fprintf(stderr, "Child process EXIT_FAILURE\n");
		exit(EXIT_FAILURE);
	}

	/* device is now up */
	printf("device is up\n", dev);


	init_ncurses();
	create_screens(&lstate);

	if(pthread_create( &tcap, NULL, device_capture_start, (void*)&lstate) != 0) {
		printf("Failed to start capture thread\n");
		exit(EXIT_FAILURE);
	}

	printw("Loki Capture | ");
	screen_refresh(lstate.current);

	input_loop(&lstate);
	screen_stop(lstate.current);

	/* bring device down */
	ifconfig_device_down(dev);
	exit(EXIT_SUCCESS);
}
static void input_loop(struct loki_state *state) {
	int code = 0;

	while((code = getch()) != 'q') {
		switch(code) {
		case 'o':
		case 27:
			state->mode == OVERVIEW;
			state->current = state->screens.overview;
			screen_refresh(state->current);
			break;
		case 'b':
			state->mode == FOCUS_AP;
			state->current = state->screens.ap;
			screen_refresh(state->current);
			break;

		}
	}
}

static void create_screens(struct loki_state *state) {
	struct screen *screen = NULL;
	struct view *vleft = NULL, *vright = NULL, *vcentre = NULL;

	vleft = create_view(2, 2, 50, LINES-4);
	vcentre = create_view(52, 2, 37, LINES-4);
	vright = create_view(89, 2, (COLS/3), LINES-4);
	scrollok(vleft->port, TRUE);
	idlok(vleft->port, TRUE);

	screen = create_screen();
	screen->left = vleft; // only create one left
	screen->centre = vcentre;
	screen->right = vright;

	state->current = state->screens.overview = screen;

	vcentre = create_view(52, 2, 37, LINES-4);
	vright = create_view(89, 2, (COLS/3), LINES-4);
	screen = create_screen();

	screen->left = vleft;
	screen->centre = vcentre;
	screen->right = vright;
	state->screens.ap = screen;

	vcentre = create_view(52, 2, 37, LINES-4);
	vright = create_view(89, 2, (COLS/3), LINES-4);
	screen = create_screen();

	screen->left = vleft;
	screen->centre = vcentre;
	screen->right = vright;
	state->screens.sta = screen;
}
