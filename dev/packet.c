#include <stdio.h>
#include <stdlib.h>
#include "packet.h"
static void print_mac_address(uint8_t *address) {

	int i = 0;
	while(i < 6) {
		if(i > 0)
			printf(" : ");
		printf("%02x", address[i++]);
	}
}

void printhdr_mac80211(struct pkth_mac80211 *mac) {
	printf("Control:\t%x\n", mac->control);
	printf("duration:\t%d ms\n", mac->duration_id);

	printf("DA:\t");
	print_mac_address(mac->da);

	printf("\nSA:\t");
	print_mac_address(mac->sa);

	printf("\nTA:\t");
	print_mac_address(mac->ta);

	printf("\nRA:\t");
	print_mac_address(mac->ra);

	printf("\nbssid:\t");
	print_mac_address(mac->bssid);
}

void printhdr_mac80211_management(struct mac80211_management_hdr *mac) {
	printf("\nbssid:\t");
	print_mac_address(mac->bssid);
	printf("\n");
}

void printhdr_etherframe(struct pkth_ethernet *frame) {
	int i = 0;
	do {
		if(frame->pre[i] != 170)
			break;
	} while(++i < 7);

	if(i < 7)
		printf("Preample invalid\n");
	else
		printf("Preamble valid\n");
}

void printraw_packet(const unsigned char *packet, unsigned int len) {
	unsigned int i = 0, j = 0;
	char *buf, *bptr;
	bptr = buf = malloc((len*3)+(len>>4)+10);

	while(i < len) {
		sprintf(bptr, "%02x ", packet[i++]);
		bptr += 3;
		if(++j == 16) {
			sprintf(bptr++, "\n");
			j = 0;
		}
	}
	printf("%s", buf);
	free(buf);
}

struct mac80211_control *decode_mac80211_control(uint16_t cf) {
	struct mac80211_control *fields = malloc(sizeof(struct mac80211_control));
	
	fields->protocol = cf&0x03;
	cf >>= 2;
	
	fields->type = cf&0x03;
	cf >>= 2;

	fields->subtype = cf&0x0f;
	cf >>= 4;

	fields->toDS = cf&0x01;
	cf >>= 1;

	fields->fromDS = cf&0x01;
	cf >>= 1;

	fields->frags = cf&0x01;
	cf >>= 1;

	fields->retry = cf&0x01;
	cf >>= 1;

	fields->powerman = cf&0x01;
	cf >>= 1;

	fields->data = cf&0x01;
	cf >>= 1;

	fields->protected = cf&0x01;
	cf >>= 1;

	fields->order = cf&0x01;
	cf >>= 1;

	return fields;
}


