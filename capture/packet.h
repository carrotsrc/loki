#ifndef CAPTUREPACKET_H
#define CAPTUREPACKET_H
struct header_ethernet {
	struct ether_addr daddr;
	struct ether_addr saddr;
	u_short type;
}__attribute__((packed));

#endif
