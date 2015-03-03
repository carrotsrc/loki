#include <pcap.h>
#include <string.h>
#include <stdlib.h>
#include "capture.h"
#include "packet.h"
/* pcap callback for reading a packet */
static void capture_cb(u_char*, const struct pcap_pkthdr*, const u_char*);

static struct beacon_frame_item *beacon_ssid_exists(struct beacon_frame_item*, char*);
static struct proberq_frame_item *proberq_ssid_exists(struct proberq_frame_item*, char*);

static void process_beacon(uint8_t*, struct mac80211_control*, uint16_t, struct frame_log*);
static void process_probe_request(uint8_t*, struct mac80211_control*, uint16_t, struct frame_log*);
static char *elements_get_ssid(uint8_t*, uint16_t);

int device_capture(const char *dev) {

	pcap_t *handle = NULL;
	char errbuf[PCAP_ERRBUF_SIZE], *filter = NULL;
	bpf_u_int32 ipaddr, netmask;
	struct bpf_program fp;
	const u_char *packet = NULL;
	struct frame_log log = (const struct frame_log){0};


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

	if(pcap_loop(handle, 0, capture_cb, (u_char*)&log) == -1) {
		fprintf(stderr, "Error on capture loop\n");
		pcap_close(handle);
		return 1;
	}
	pcap_close(handle);
	return 0;

}


void capture_cb(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {

	uint16_t eth_begin = 0, sz = 0, hsize = 0;

	struct mac80211_management_hdr *manhdr = NULL;
	struct mac80211_control *mctrl = NULL;
	struct frame_log *log = NULL;
	
	
	log = (struct frame_log*) args;

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
	//printraw_management_frame(packet, header->len);
	//printf("-----------\n\n\n");
	free(mctrl);

}

static struct beacon_frame_item *beacon_ssid_exists(struct beacon_frame_item *list, char *value) {
	if(list == NULL)
		return NULL;

	do {
		if(strcmp(list->ssid, value) == 0)
			return list;
	} while((list = list->next) != NULL);
	return NULL;
}

static struct proberq_frame_item *proberq_ssid_exists(struct proberq_frame_item *list, char *value) {
	if(list == NULL)
		return NULL;

	do {
		if(strcmp(list->ssid, value) == 0)
			return list;
	} while((list = list->next) != NULL);
	return NULL;
}

static void process_beacon(uint8_t *frame, struct mac80211_control *mctrl, uint16_t len, struct frame_log *log) {

	struct mac80211_beacon_fixed *beacon_fixed = NULL;
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
	} else {
		free(ssid);
	}

	item->count++;
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
				ssid = (char*) malloc(sizeof(char)*2);
				ssid = "*\0";
			} else {
				ssid = (char*) malloc(sizeof(char) * e[1]+1);
				memcpy(ssid, ptr, e[1]);
				ssid[e[1]] = '\0';
			}
			return ssid;
		}

		ptr += e[1];

	} while(ptr <= end);

	return NULL;

}

static void process_probe_request(uint8_t *frame, struct mac80211_control *mctrl, uint16_t len, struct frame_log *log) {

	struct mac80211_beacon_fixed *beacon_fixed = NULL;
	short drop = 4;
	if(mctrl->order)
		drop = 0;

	short hsize = sizeof(struct mac80211_management_hdr)-drop;

	char *ssid = elements_get_ssid((uint8_t*)frame+hsize, (len-hsize));
	if(ssid == NULL || ssid[0] == '*')
		return;

	struct proberq_frame_item *item = NULL;

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
		//memcpy(&(item->mac), (uint8_t*)((struct mac80211_management_hdr*)frame)->bssid, 6);
		item->count = 0;
		item->next = NULL;
		log->proberq.num++;
	} else {
		free(ssid);
	}

	item->count++;
	printf("Request for: %s\nRequest num: %ld\n", item->ssid, item->count);
}
