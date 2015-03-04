#ifndef PACKET_H
#define PACKET_H
#include <stdint.h>
#include <ncurses.h>
struct pkth_radiotap {
	uint8_t version;
	uint8_t pad;
	uint16_t len;
	uint32_t present;
} __attribute__((__packed__));

struct pkth_mac80211 {
	uint16_t control;
	uint16_t duration_id;
	uint8_t da[6]; ///< Destination MAC
	uint8_t sa[6]; ///< Source MAC
	uint8_t ta[6]; ///< Transmitter MAC
	uint8_t ra[6]; ///< Receiver MAC
	uint8_t bssid[6]; ///< Basic Service Set ID
} __attribute__((__packed__));

struct mac80211_management_hdr {
	uint16_t control;
	uint16_t duration_id;
	uint8_t ra[6]; ///< Destination MAC
	uint8_t ta[6]; ///< Transmitter MAC
	uint8_t bssid[6]; ///< Basic Service Set ID
	uint16_t seqctrl; ///< Sequence Control
	uint32_t htctrl; ///< HT Control - present if control.order = 1
} __attribute__((__packed__));

struct mac80211_beacon_fixed {
	uint8_t timestamp[8]; ///< Timestamp
	uint16_t beacon_interval; ///< period between beacon broadcasts
	uint16_t cap_info; ///< capabilities of STA
} __attribute__((__packed__));

struct pkth_ethernet {
	uint8_t pre[7]; ///< Preamble
	uint8_t delim; ///< Delimiter
	uint8_t dst[6]; ///< Destination MAC
	uint8_t src[6]; ///< Source MAC
	uint16_t type;
} __attribute__((__packed__));

enum mac80211_control_type {
	MANAGEMENT,
	CONTROL,
	DATA
};

enum mac80211_subtype_management {
	ASSOC_REQUEST,
	ASSOC_RESPONSE,
	REASSOC_REQUEST,
	REASSOC_RESPONSE,
	PROBE_REQUEST,
	PROBE_RESPONSE,
	TIMING_AD,
	RESERVED_A,
	BEACON,
	ATIM,
	DISASSOC,
	AUTH,
	DEAUTH,
	ACTION,
	ACTION_NO_ACK,
	RESERVED_B
};

struct mac80211_control {
	uint8_t protocol;
	enum mac80211_control_type type;
	enum mac80211_subtype_management subtype;
	uint8_t toDS;
	uint8_t fromDS;
	uint8_t frags;
	uint8_t retry;
	uint8_t powerman;
	uint8_t data;
	uint8_t protected;
	uint8_t order;

};

void printraw_packet(const unsigned char*, unsigned int, WINDOW *handle);
void printraw_management_frame(const uint8_t*, uint16_t, WINDOW*);
void print_mac_address(uint8_t*);
struct mac80211_control *decode_mac80211_control(uint16_t);
#endif
