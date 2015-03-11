#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "packet.h"
char *printraw_packet(const unsigned char *packet, unsigned int len) {
	unsigned int i = 0, j = 0;
	char *buf, *bptr;
	bptr = buf = malloc((len*3)+(len>>4)+11);

	while(i < len) {
		sprintf(bptr, "%02x ", packet[i++]);
		bptr += 3;
		if(++j == 16) {
			sprintf(bptr++, "\n");
			j = 0;
		}
	}
	bptr = '\0';
	return buf;
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

char *printraw_management_frame(const uint8_t *packet, uint16_t len) {
	uint8_t mac_begin, hsize;
	size_t fsize;
	struct mac80211_management_hdr *manhdr = NULL;
	struct mac80211_control *mctrl = NULL;
	char *sradio, *smanhdr, *smanframe, *formatted;

	mac_begin = ((struct pkth_radiotap*)packet)->len;
	len -= mac_begin;

	// Print radiotap header
	sradio = printraw_packet((uint8_t*)packet, mac_begin);

	manhdr = (struct mac80211_management_hdr*) ((uint8_t*)packet+mac_begin);
	mctrl = decode_mac80211_control(manhdr->control);
	hsize = sizeof(struct mac80211_management_hdr);
	if(mctrl->order == 0) hsize -= 4; // We drop the 4-byte HT field
	len -= hsize;

	// print managament header
	smanhdr = printraw_packet((uint8_t*)manhdr, hsize);

	// print frame body
	smanframe = printraw_packet((uint8_t*)manhdr+hsize, len);

	fsize = strlen(sradio) + strlen(smanhdr) + strlen(smanframe) + 6;
	formatted = (char*)malloc(sizeof(char)*fsize);

	sprintf(formatted, "%s\n\n%s\n\n%s\n", sradio, smanhdr, smanframe);
	free(sradio);
	free(smanhdr);
	free(smanframe);
	return formatted;
}

char *print_mac_address(uint8_t *address) {
	char *faddr, *floc;

	faddr  = (char*)malloc(sizeof(char)*32);
	floc = faddr;
	int i = 0;

	while(i < 6) {
		if(i > 0) {
			sprintf(floc, ":");
			floc += 1;
		}
		sprintf(floc, "%02x", address[i++]);
		floc += 2;
	}

	floc = '\0';
	return faddr;
}

uint16_t encode_mac80211_control(const struct mac80211_control *stc) {
	uint16_t ctrl;
	ctrl |= stc->order;
	ctrl <<= 1;

	ctrl |= stc->protected;
	ctrl <<= 1;

	ctrl |= stc->data;
	ctrl <<= 1;

	ctrl |= stc->powerman;
	ctrl <<= 1;

	ctrl |= stc->retry;
	ctrl <<= 1;

	ctrl |= stc->frags;
	ctrl <<= 1;

	ctrl |= stc->fromDS;
	ctrl <<= 1;

	ctrl |= stc->toDS;
	ctrl <<= 4;

	ctrl |= stc->subtype;
	ctrl <<= 2;

	ctrl |= stc->type;
	ctrl <<= 2;

	ctrl |= stc->protocol;
	return ctrl;
}

struct header_radiotap *construct_header_radiotap() {
	struct header_radiotap *header;
	
	header = (struct header_radiotap*) malloc(sizeof(struct header_radiotap));

	header->version = 0;
	header->pad = 0;
	header->len = sizeof(struct header_radiotap);
	header->present = 0;
	return header;
}

struct mac80211_management_hdr *construct_header_management(const uint8_t *ta, const uint8_t *ra, const uint8_t *bssid, const struct mac80211_control *stc) {
	struct mac80211_management_hdr *mng = (struct mac80211_management_hdr*)malloc(sizeof(struct mac80211_management_hdr));
	mng->control = encode_mac80211_control(stc);
	mng->duration_id = 500;
	memcpy(mng->ra, ra, 6);
	memcpy(mng->ta, ta, 6);
	memcpy(mng->bssid, bssid, 6);
	mng->seqctrl = 0;
	return mng;
}

uint8_t *construct_packet(struct header_radiotap *tap, struct mac80211_management_hdr *header, uint16_t reason, size_t *len) {
	uint8_t *packet = (uint8_t*) malloc( tap->len + sizeof(struct mac80211_management_hdr) + 2);
	size_t pos = 0;

	memcpy(packet+pos, tap, tap->len);
	pos += tap->len;

	memcpy(packet+pos, header, sizeof(struct mac80211_management_hdr));
	pos += sizeof(struct mac80211_management_hdr);

	memcpy(packet+pos, &reason, 2);
	pos += 2;
	*len = pos;
	return packet;
}
