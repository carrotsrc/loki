#include <string.h>
#include <stdlib.h>

#include "frameproc.h"

static uint8_t filter_frame_management(const uint8_t*, uint16_t, const struct mac80211_control*, struct loki_state*);
static uint8_t filter_frame_data(const uint8_t*, uint16_t, const struct mac80211_control*, struct loki_state*);

static struct beacon_ssid_item *beacon_ssid_exists(struct beacon_ssid_item*, const char*);
static struct proberq_frame_item *proberq_ssid_exists(struct proberq_frame_item*, const char*);

static struct macaddr_list_item *proberq_mac_exists(struct macaddr_list_item*, uint8_t*);
static struct beacon_frame_item *beacon_mac_exists(struct beacon_ssid_item*, uint8_t*);
static struct beacon_frame_item *beacon_ap_exists(const char*, const struct beacon_frame_item*);

static char *elements_get_ssid(uint8_t*, uint16_t);


static unsigned int process_beacon(const uint8_t*, const struct mac80211_control*, uint16_t, struct frame_log*);
static int process_beacon_ap(const struct mac80211_management_hdr*, struct beacon_ssid_item*);
static unsigned int process_probe_request(const uint8_t*, const struct mac80211_control*, uint16_t, struct frame_log*);

static unsigned int process_data(const uint8_t*, const struct mac80211_control*, uint16_t, struct frame_log*);

uint8_t filter_frame(const uint8_t *packet, uint16_t len, struct loki_state *state) {
	uint8_t ret;
	uint16_t eth_begin = 0, sz = 0, hsize = 0;

	struct mac80211_management_hdr *manhdr = NULL;
	struct mac80211_control *mctrl = NULL;

	eth_begin = ((struct pkth_radiotap*)packet)->len;
	sz = len - eth_begin;

	manhdr = (struct mac80211_management_hdr*) (packet+eth_begin);
	mctrl = decode_mac80211_control(manhdr->control);
	
	switch(mctrl->type) {
	case MANAGEMENT:
		ret = filter_frame_management((uint8_t*)(packet+eth_begin), sz, mctrl, state);
		break;
	
	case DATA:
		filter_frame_data((uint8_t*)(packet+eth_begin), sz, mctrl, state);
		ret = 0; // don't write the packet
		break;
	
	default:
		ret = 0;
		break;

	}

	free(mctrl);

	return ret;
}

static struct beacon_ssid_item *beacon_ssid_exists(struct beacon_ssid_item *list, const char *value) {
	if(list == NULL)
		return NULL;

	do {
		if(strcmp(list->ssid, value) == 0)
			return list;
	} while((list = list->next) != NULL);

	return NULL;
}

static struct proberq_frame_item *proberq_ssid_exists(struct proberq_frame_item *list, const char *value) {
	if(list == NULL)
		return NULL;

	do {
		if(strcmp(list->ssid, value) == 0)
			return list;
	} while((list = list->next) != NULL);
	return NULL;
}

static struct macaddr_list_item *proberq_mac_exists(struct macaddr_list_item *list, uint8_t *value) {
	if(list == NULL)
		return NULL;

	do {
		if(memcmp(list->addr, value, 6) == 0)
			return list;
	} while((list = list->next) != NULL);
	return NULL;
}

static unsigned int process_beacon(const uint8_t *frame, const struct mac80211_control *mctrl, uint16_t len, struct frame_log *log) {

	struct mac80211_beacon_fixed *beacon_fixed = NULL;
	unsigned int modified = 0;
	short drop = 4;

	if(mctrl->order)
		drop = 0;

	short hsize = sizeof(struct mac80211_management_hdr)-drop;

	beacon_fixed = (struct mac80211_beacon_fixed*) ((uint8_t*)frame + hsize);
	char *ssid = elements_get_ssid((uint8_t*)beacon_fixed + sizeof(struct mac80211_beacon_fixed), (len-hsize + sizeof(struct mac80211_beacon_fixed)));
	if(ssid == NULL)
		return 0;

	struct beacon_ssid_item *item = NULL;

	if( (item = beacon_ssid_exists(log->beacon.list, ssid)) == NULL) {
		item = (struct beacon_ssid_item*) malloc(sizeof(struct beacon_ssid_item));

		if(log->beacon.list == NULL)
			log->beacon.list = item;

		item->prev = log->beacon.tail;
		if(item->prev != NULL)
			item->prev->next = item;

		log->beacon.tail = item;
		item->ssid_len = strlen(ssid);
		item->ssid = ssid;
		item->bss_count = 0;
		item->selected = 0;
	
		// linked list
		item->next = NULL;

		// APs addresses
		item->list = NULL;
		item->tail = NULL;
		
		process_beacon_ap((struct mac80211_management_hdr*)frame, item);
		log->beacon.num++;
		modified = 1;
	} else {
		modified = process_beacon_ap((struct mac80211_management_hdr*)frame, item);
		free(ssid);
	}

	//item->count++;
	return modified;
}

int process_beacon_ap(const struct mac80211_management_hdr *hdr, struct beacon_ssid_item *ess) {
	struct beacon_frame_item* item = NULL;
	if( (item = beacon_ap_exists(hdr->bssid, ess->list)) == NULL ) {
		item = (struct beacon_frame_item*)malloc(sizeof(struct beacon_frame_item));
		
		if(ess->list == NULL)
			ess->list = item;

		item->prev = ess->tail;
		if(item->prev != NULL)
			item->prev->next = item;

		ess->tail = item;
		memcpy(&(item->mac), (uint8_t*)(hdr->bssid), 6);
		item->count = 1;
		item->sta_count = 0;
		item->sta_selected = 0;
		
		
		item->next = NULL;
		
		item->list = NULL;
		item->tail = NULL;
		ess->bss_count++;
		return 1;
		
	} else {
		item->count++;
	}
	
	return 0;
}

static struct beacon_frame_item *beacon_ap_exists(const char* value, const struct beacon_frame_item* item) {
	if(item == NULL) return NULL;
	
	do {
		if(memcmp(item->mac, value, 6)  == 0)
			return item;
			
	} while((item = item->next));
	
	return NULL;
}

static char *elements_get_ssid(uint8_t *elements, uint16_t len) {
	uint8_t e[2], *ptr, *end;
	end = elements+len;
	ptr = elements;
	char *ssid;

	do {
		e[0] = *(ptr++);
		e[1] = *(ptr++);

		if(e[0] == 0) {
			if(e[1] == 0) {
				while((ssid = (char*) malloc(sizeof(char)*2)) == NULL)
					continue;
				strcpy(ssid, "*\0");
			} else {
				while((ssid = (char*) malloc(sizeof(char) * e[1]+1)) == NULL)
					continue;
				memcpy(ssid, ptr, e[1]);
				ssid[e[1]] = '\0';
			}
			return ssid;
		}

		ptr += e[1];

	} while(ptr <= end);

	return NULL;

}

static unsigned int process_probe_request(const uint8_t *frame, const struct mac80211_control *mctrl, uint16_t len, struct frame_log *log) {

	struct mac80211_beacon_fixed *beacon_fixed = NULL;
	unsigned int modified = 0;
	short drop = 4;
	if(mctrl->order)
		drop = 0;

	short hsize = sizeof(struct mac80211_management_hdr)-drop;

	char *ssid = elements_get_ssid((uint8_t*)frame+hsize, (len-hsize));
	if(ssid == NULL)
		return;

	if(ssid[0] == '*') {
		free(ssid);
		return;
	}

	struct proberq_frame_item *item = NULL;

	// Add any new SSIDs to the list of requests
	if( (item = proberq_ssid_exists(log->proberq.list, ssid)) == NULL) {
		item = (struct proberq_frame_item*) malloc(sizeof(struct proberq_frame_item));

		if(log->proberq.list == NULL)
			log->proberq.list = item;

		item->prev = log->proberq.tail;
		if(item->prev != NULL)
			item->prev->next = item;

		log->proberq.tail = item;
		item->ssid_len = strlen(ssid);
		item->ssid = ssid;
		item->list = item->tail = NULL;
		item->count = 0;
		item->next = NULL;
		log->proberq.num++;
		modified  = 1;
	} else {
		free(ssid);
	}

	item->count++;

	// add any new trasmitter addresses for the SSID probe request
	uint8_t *addr = ((struct mac80211_management_hdr*)frame)->ta;
	if(proberq_mac_exists(item->list, addr) == NULL ) {
		struct macaddr_list_item *addr_item = (struct macaddr_list_item*) malloc(sizeof(struct macaddr_list_item));
		memcpy(addr_item->addr, addr, 6);
		
		if(item->list == NULL)
			item->list = addr_item;

		if(item->tail != NULL)
			item->tail->next = addr_item;

		addr_item->prev = item->tail;
		addr_item->next = NULL;
	}

	return modified;
}

static uint8_t filter_frame_management(const uint8_t *packet, uint16_t len, const struct mac80211_control *mctrl, struct loki_state *state) {
	struct mac80211_management_hdr *manhdr = NULL;

	manhdr = (struct mac80211_management_hdr*)packet;
	switch(mctrl->subtype) {
	case BEACON:
		process_beacon((uint8_t*)manhdr, mctrl, len, state->log);
		break;

	case PROBE_REQUEST:
		process_probe_request((uint8_t*)manhdr, mctrl, len, state->log);
		break;
	default:
		return 0;
	}

	return 1;
}

static uint8_t filter_frame_data(const uint8_t *packet, uint16_t len, const struct mac80211_control *mctrl, struct loki_state *state) {

	process_data(packet, mctrl, len, state->log);

	return 0;
}

static unsigned int process_data(const uint8_t *frame, const struct mac80211_control *mctrl, uint16_t len, struct frame_log *log) {
	struct mac80211_data *datahdr = NULL;
	struct beacon_frame_item *item;
	struct macaddr_list_item *maddr = NULL;

	datahdr = (struct mac80211_data*)frame;
	if(mctrl->fromDS == 1 && mctrl->toDS == 0) {
		//printw("From distribution system");
	} else
	if(mctrl->fromDS == 0 && mctrl->toDS == 1) {
		if((item = beacon_mac_exists(log->beacon.list, datahdr->ra)) != NULL) {
			if(proberq_mac_exists(item->list, datahdr->sa) == NULL) {

				maddr = (struct macaddr_list_item*) malloc(sizeof(struct macaddr_list_item));
				if(item->sta_count == 0) {
					item->list = maddr;
					item->sta_selected = 0;
				}
				maddr->prev = item->tail;
				maddr->next = NULL;

				if(item->tail != NULL)
					item->tail->next = maddr;

				item->tail = maddr;
				memcpy(maddr->addr, datahdr->sa, 6);
				item->sta_count++;
			}
		}
	}
}

static struct beacon_frame_item *beacon_mac_exists(struct beacon_ssid_item *list, uint8_t *value) {
	if(list == NULL)
		return NULL;
	struct beacon_frame_item* aplist = NULL;
	
	do {
		aplist = list->list;
		
		do {
			if(memcmp(aplist->mac, value, 6) == 0)
				return aplist;
		} while((aplist = aplist->next) != NULL);
	} while( (list = list->next));
	
	return NULL;
}
