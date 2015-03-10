#ifndef PACKET_H
#define PACKET_H
#include <stdint.h>

enum mac80211_subtype_management {
	ASSOC_REQUEST,
	ASSOC_RESPONSE,
	REASSOC_REQUEST,
	REASSOC_RESPONSE,
	PROBE_REQUEST,
	PROBE_RESPONSE,
	TIMING_AD,
	MANAGEMENT_RESERVED_A,
	BEACON,
	ATIM,
	DISASSOC,
	AUTH,
	DEAUTH,
	ACTION,
	ACTION_NO_ACK,
	MANAGEMENT_RESERVED_B
};

enum mac80211_control_type {
	MANAGEMENT,
	CONTROL,
	DATA
};

struct header_radiotap {
	uint8_t version;
	uint8_t pad;
	uint16_t len;
	uint32_t present;
}__attribute__((packed));

struct header_management {
	uint16_t control;
	uint16_t duration_id;
	uint8_t ra[6]; ///< Destination MAC
	uint8_t ta[6]; ///< Transmitter MAC
	uint8_t bssid[6]; ///< Basic Service Set ID
	uint16_t seqctrl; ///< Sequence Control
} __attribute__((__packed__));

struct mac80211_control {
	uint8_t protocol;
	uint8_t type;
	uint8_t subtype;
	uint8_t toDS;
	uint8_t fromDS;
	uint8_t frags;
	uint8_t retry;
	uint8_t powerman;
	uint8_t data;
	uint8_t protected;
	uint8_t order;

};

struct header_radiotap *construct_header_radiotap();
struct header_management *construct_header_management(const uint8_t*, const uint8_t*, const uint8_t*, const struct mac80211_control*);
uint16_t encode_mac80211_control(const struct mac80211_control*);
#endif
