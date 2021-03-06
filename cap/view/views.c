#include <string.h>
#include <stdlib.h>

#include "views.h"
#include "capture.h"
#include "controller/controllers.h"

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



void print_overview_left(struct loki_state *state, WINDOW *handle) {
	char *packet = NULL;
	if(( packet = state->log->lastPacket) != NULL) {
		wprintw(handle, "%s\n---------\n\n", packet);
		free(packet);
		((struct frame_log*)state->log)->lastPacket = NULL;
	}
}

void print_overview_centre(struct loki_state *state, WINDOW *handle) {
	wmove(handle, 0 , 0);
	wprintw(handle, "Beacon Frames\n-------------\nTotal: \n\n");

	struct beacon_ssid_item *item = ((struct frame_log*)state->log)->beacon.list;
	if(item == NULL)
		return;
	int i = 0;
	do {
		if(i++ == state->log->beacon.selected)
			wprintw(handle, "> %s\n", item->ssid);
		else
			wprintw(handle, "  %s\n", item->ssid);
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

	int i = 0;
	do {
		if(i++ == state->log->proberq.selected)
			wprintw(handle, "> %ld\t%s\n", item->count, item->ssid);
		else
			wprintw(handle, "  %ld\t%s\n", item->count, item->ssid);

	} while( (item = item->next) != NULL);

	wmove(handle, 2, 8);
	wprintw(handle, "%ld", ((struct frame_log*)state->log)->proberq.num);

}

void print_ap_left(struct loki_state *state, WINDOW *handle) {
}

void print_ap_centre(struct loki_state *state, WINDOW *handle) {
	struct beacon_ssid_item *item = state->log->beacon.list;
	struct beacon_frame_item *apitem =NULL;
	int i = 0;
	do {
		if(i++ == state->log->beacon.selected)
			break;
	} while( (item = item->next) != NULL);

	wmove(handle, 0 , 0);
	wprintw(handle, "BSS AcPt List\n");
	wprintw(handle, "-------------\n");
	
	i = 0;
	apitem = item->list;
	do {
		if(item->selected == i) {
			wprintw(handle, "> %s\n", print_mac_address(apitem->mac));
		} else {
			wprintw(handle, "  %s\n", print_mac_address(apitem->mac));
		}
		++i;
	} while ((apitem = apitem->next));

}

void print_ap_right(struct loki_state *state, WINDOW *handle) {

	struct beacon_ssid_item *item = state->log->beacon.list;
	struct beacon_frame_item *apitem = NULL;
	struct macaddr_list_item *addr;
	int i = 0;
	do {
		if(i++ == state->log->beacon.selected) {
			i = 0;
			apitem = item->list;
			do {
				if(i++ == item->selected)
					break;
			} while( (apitem = apitem->next) );
		}
	} while( (item = item->next) != NULL);

	wmove(handle, 0 , 0);
	wprintw(handle, "Stations\n");
	wprintw(handle, "--------\n");

	addr = apitem->list;
	i = 0;
	if((addr = apitem->list) == NULL)
		return;

	do {
		if(i++ == apitem->sta_selected && state->controllers.ap->selected == state->controllers.ap->right)
			wprintw(handle, "> %s\n", print_mac_address(addr->addr));
		else
			wprintw(handle, "  %s\n", print_mac_address(addr->addr));
	} while( (addr = addr->next) != NULL);

}

void print_sta_centre(struct loki_state *state, WINDOW *handle) {
	struct macaddr_list_item *mitem;
	struct proberq_frame_item *item = state->log->proberq.list;
	int i = 0;
	do {
		if(i++ == state->log->proberq.selected)
			break;
	} while( (item = item->next) != NULL);

	wmove(handle, 0 , 0);
	wprintw(handle, "Probes\n");
	wprintw(handle, "------\n");
	wprintw(handle, "%s\n", item->ssid);
	mitem = item->list;
	do {
		wprintw(handle, "%s\n", print_mac_address(mitem->addr));
	} while( (mitem = mitem->next) != NULL);
}

void print_sta_right(struct loki_state *state, WINDOW *handle) {
	wmove(handle, 0 , 0);
	wprintw(handle, "STA RIGHT");
}
