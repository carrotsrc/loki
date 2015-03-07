#include <pcap.h>
#include "capture.h"
#include "packet.h"

static int device_capture(struct loki_state *);

/* pcap callback for reading a packet */
static void capture_cb(u_char*, const struct pcap_pkthdr*, const u_char*);


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

	//filter = "broadcast";
	filter = "type mgt or type data";
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
	struct frame_log *log = NULL;
	


	state = (struct loki_state*) args;
	log = (struct frame_log*) state->log;
	log->totalPackets++;

	if(!filter_frame(packet, header->len, state))
		return;

	packetCheck++;
	if( (packetCheck%50) == 0 ) {
		wattron(state->screens.overview->left->port, COLOR_PAIR(1));
		packetCheck = 0;
		char *formatted = printraw_management_frame(packet, header->len);
		log->lastPacket = formatted;
	}
	write_screen(state->current, state);

}

