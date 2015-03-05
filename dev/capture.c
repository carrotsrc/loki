#include <pcap.h>
#include <string.h>
#include <stdlib.h>

#include "capture.h"
#include "packet.h"

static int device_capture(struct loki_state *);

/* pcap callback for reading a packet */
static void capture_cb(u_char*, const struct pcap_pkthdr*, const u_char*);

static struct beacon_frame_item *beacon_ssid_exists(struct beacon_frame_item*, const char*);
static struct proberq_frame_item *proberq_ssid_exists(struct proberq_frame_item*, const char*);
static struct macaddr_list_item *proberq_mac_exists(struct macaddr_list_item*, uint8_t*);
static char *elements_get_ssid(uint8_t*, uint16_t);


static unsigned int process_beacon(uint8_t*, struct mac80211_control*, uint16_t, struct frame_log*);
static unsigned int process_probe_request(uint8_t*, struct mac80211_control*, uint16_t, struct frame_log*);

void *device_capture_start(void *data) {
	device_capture((struct loki_state*)data);
}



int device_capture(struct loki_state *state) {

	pcap_t *handle = NULL;
	char errbuf[PCAP_ERRBUF_SIZE], *filter = NULL, *dev;
	bpf_u_int32 ipaddr, netmask;
	struct bpf_program fp;
	const u_char *packet = NULL;
	struct frame_log log = (const struct frame_log){0};

	dev = state->dev;
	state->log = (void*)&log;


	if(pcap_lookupnet(dev, &ipaddr, &netmask, errbuf) == -1) {
		fprintf(stderr, "Error on device lookup\n%s\n", errbuf);
		return 1;
	}

	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if(handle == NULL) {
		fprintf(stderr, "Couldn't open device %s", errbuf);
		return 1;
	}

	filter = "broadcast";
	if(pcap_compile(handle, &fp, filter, 0, 0) == -1) {
		fprintf(stderr, "Couldn't compiler filter / %s\n", pcap_geterr(handle));
		return 1;
	}

	if(pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter\n", pcap_geterr(handle));
		return 1;
	}

	if(pcap_loop(handle, 0, capture_cb, (u_char*)state) == -1) {
		fprintf(stderr, "Error on capture loop\n");
		pcap_close(handle);
		return 1;
	}
	pcap_close(handle);
	return 0;

}


void capture_cb(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {

	static int packetCheck = 0;
	static long long totalPackets = 0;
	uint16_t eth_begin = 0, sz = 0, hsize = 0;

	struct loki_state *state;
	struct mac80211_management_hdr *manhdr = NULL;
	struct mac80211_control *mctrl = NULL;
	struct frame_log *log = NULL;
	


	state = (struct loki_state*) args;
	log = (struct frame_log*) state->log;
	log->totalPackets++;

	eth_begin = ((struct pkth_radiotap*)packet)->len;
	sz = header->len - eth_begin;

	manhdr = (struct mac80211_management_hdr*) (packet+eth_begin);
	mctrl = decode_mac80211_control(manhdr->control);

	switch(mctrl->subtype) {
	case BEACON:
		process_beacon((uint8_t*)manhdr, mctrl, sz, log);
		break;

	case PROBE_REQUEST:
		process_probe_request((uint8_t*)manhdr, mctrl, sz, log);
		break;
	default:
		free(mctrl);
		return;
	}

	packetCheck++;
	if( (packetCheck%50) == 0 ) {
		wattron(state->screens.overview->left->port, COLOR_PAIR(1));
		packetCheck = 0;
		char *formatted = printraw_management_frame(packet, header->len);
		log->lastPacket = formatted;
	}
	free(mctrl);
	write_screen(state->current, state);

}

static struct beacon_frame_item *beacon_ssid_exists(struct beacon_frame_item *list, const char *value) {
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

static unsigned int process_beacon(uint8_t *frame, struct mac80211_control *mctrl, uint16_t len, struct frame_log *log) {

	struct mac80211_beacon_fixed *beacon_fixed = NULL;
	unsigned int modified = 0;
	short drop = 4;
	if(mctrl->order)
		drop = 0;

	short hsize = sizeof(struct mac80211_management_hdr)-drop;

	beacon_fixed = (struct mac80211_beacon_fixed*) ((uint8_t*)frame + hsize);
	char *ssid = elements_get_ssid((uint8_t*)beacon_fixed + sizeof(struct mac80211_beacon_fixed), (len-hsize + sizeof(struct mac80211_beacon_fixed)));
	if(ssid == NULL)
		return;

	struct beacon_frame_item *item = NULL;

	if( (item = beacon_ssid_exists(log->beacon.list, ssid)) == NULL) {
		item = (struct beacon_frame_item*) malloc(sizeof(struct beacon_frame_item));

		if(log->beacon.list == NULL)
			log->beacon.list = item;

		item->prev = log->beacon.tail;
		if(item->prev != NULL)
			item->prev->next = item;

		log->beacon.tail = item;
		item->ssid_len = strlen(ssid);
		item->ssid = ssid;
		memcpy(&(item->mac), (uint8_t*)((struct mac80211_management_hdr*)frame)->bssid, 6);
		item->count = 0;
		item->next = NULL;
		log->beacon.num++;
		modified = 1;
	} else {
		free(ssid);
	}

	item->count++;
	return modified;
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

static unsigned int process_probe_request(uint8_t *frame, struct mac80211_control *mctrl, uint16_t len, struct frame_log *log) {

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
