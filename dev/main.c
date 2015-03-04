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



int main( int argc, char *argv[]) {

	pthread_t tcap, tui;
	struct screen *screen = NULL;
	struct view *vleft = NULL, *vright = NULL, *vcentre = NULL;
	struct loki_state lstate;
	char *addr, *dev;
	int r;

	cbreak();

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

	printf("device is up\n", dev);
	/* device is now up */

	if(pthread_create( &tcap, NULL, device_capture_start, (void*)&lstate) != 0) {
		printf("Failed to start capture thread\n");
		exit(EXIT_FAILURE);
	}


	screen = screen_start();

	vleft = create_view(0, 2, (COLS/3), LINES-4);
	screen->left = vleft;

	printw("Loki Capture");
	scrollok(vleft->port, TRUE);
	idlok(vleft->port, TRUE);
	wmove(vleft->port, 1, 1);
	screen_refresh(screen);

	int i  = 0;
	do {
		wprintw(vleft->port, "Raw Feed %d\n", i);
		view_refresh(vleft);
	} while(i++ < 100001);


	/* bring device down */
	getch();
	screen_stop(screen);
	ifconfig_device_down(dev);
	exit(EXIT_SUCCESS);
}


