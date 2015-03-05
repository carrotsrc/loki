#include "views.h"
#include "capture.h"
#include <stdlib.h>

struct view *create_view(uint16_t x, uint16_t y, uint16_t w, uint16_t h, void (*cb)(struct loki_state*,WINDOW*)) {

	struct view *view = (struct view*) malloc(sizeof(struct view));
	view->x = x; 
	view->y = y; 
	view->w = w; 
	view->h = h;
	view->write = cb;

	view->port = newwin(h,w,y,x);

	return view;
}

void view_refresh(struct view *view) {
	wnoutrefresh(view->port);
}

char *print_mac_address(uint8_t *address) {
	char *faddr, *floc;

	faddr  = (char*)malloc(sizeof(char)*32);
	floc = faddr;
	int i = 0;

	while(i < 6) {
		if(i > 0) {
			sprintf(floc, " : ");
			floc += 3;
		}
		sprintf(floc, "%02x", address[i++]);
		floc += 2;
	}

	floc = '\0';
	return faddr;
}


void print_overview_left(struct loki_state *state, WINDOW *handle) {
	char *packet = NULL;
	if(( packet = ((struct frame_log*)state->log)->lastPacket) != NULL) {
		wprintw(handle, "%s\n---------\n\n", packet);
		free(packet);
		((struct frame_log*)state->log)->lastPacket = NULL;
	}
}

void print_overview_centre(struct loki_state *state, WINDOW *handle) {
	wmove(handle, 0 , 0);
	wprintw(handle, "Beacon Frames\n-------------\nTotal: \n\n");

	struct beacon_frame_item *item = ((struct frame_log*)state->log)->beacon.list;
	if(item == NULL)
		return;

	do {
		wprintw(handle, "%s\n", item->ssid);
	} while( (item = item->next) != NULL);

	wmove(handle, 2, 8);
	wprintw(handle, "%ld", ((struct frame_log*)state->log)->beacon.num);
}

void print_overview_right(struct loki_state *state, WINDOW *handle) {
	wmove(handle, 0 , 0);
	wprintw(handle, "Probe Requests\n--------------\nTotal: \n\n");
	struct proberq_frame_item *item = ((struct frame_log*)(state->log))->proberq.list;
	if(item == NULL)
		return;

	do {
		wprintw(handle, "%ld\t%s\n", item->count, item->ssid);
	} while( (item = item->next) != NULL);

	wmove(handle, 2, 8);
	wprintw(handle, "%ld", ((struct frame_log*)state->log)->proberq.num);

}
