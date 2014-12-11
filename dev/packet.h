#ifndef PACKET_H
#define PACKET_H
#include <stdint.h>
struct pkth_radiotap {
	uint8_t version;
	uint8_t pad;
	uint16_t len;
	uint32_t present;
} __attribute__((__packed__));

struct pkth_mac80211 {
	uint16_t control;
	uint16_t duration_id;
	uint8_t dst[6];
	uint8_t src[6];
	uint8_t trn[6];
	uint8_t rcv[6];
	uint8_t bssid[6];
} __attribute__((__packed__));

struct pkth_ethernet {
	uint8_t pre[7];
	uint8_t delim;
	uint8_t dst[6];
	uint8_t src[6];
	uint16_t type;
} __attribute__((__packed__));

void printhdr_mac80211(struct pkth_mac80211*);
void printraw_packet(const unsigned char*, unsigned int);
#endif
