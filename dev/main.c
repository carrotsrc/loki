#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/genl/genl.h>
#include <netinet/if_ether.h>

#include <pcap.h>

#include "nl80211.h"

void init(const char *dev, struct nl_sock *nls, int *family, signed long long *devid) {
	if(!nls) {
		fprintf(stderr, "Failed to allocate socket\n");
		exit(EXIT_FAILURE);
	}

	nl_socket_set_buffer_size(nls, 8192, 8192);
	/* this creates a local socket + binds to GENERIC NETLINK*/
	if(genl_connect(nls)) {
		fprintf(stderr, "Failed to create socket fd\n");
		nl_socket_free(nls);
		exit(EXIT_FAILURE);
	}

	/* the resolvers convert family names into unqiue IDs*/
	*family = genl_ctrl_resolve(nls, "nl80211");

	if(*family < 0) {
		fprintf(stderr, "Failed to resolve ID for nl80211 family\n");
		nl_socket_free(nls);
		exit(EXIT_FAILURE);
	}


	*devid = if_nametoindex(dev);
}

static int ack_cb(struct nl_msg *msg, void *arg) {
	printf("ack\n");
	*((int*)arg)  = 0;
	return NL_STOP;
}

static int finish_cb(struct nl_msg *msg, void *arg) {
	printf("fin\n");
	*((int*)arg)  = 0;
	return NL_SKIP;
}

static int error_cb(struct nl_msg *msg, struct nlmsgerr *err, void *arg) {
	printf("err\n");
	*((int*)arg)  = 0;
	return NL_SKIP;
}

int main( int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Device unspecified\n");
		exit(EXIT_FAILURE);
	}

	char *dev = argv[1], errbuf[PCAP_ERRBUF_SIZE];
	struct nl_msg *msg = NULL;
	struct nl_sock *nls = nl_socket_alloc();
	struct nl_cb *cb = NULL;
	signed long long devid;
	int family, cmd, bytes, rcode = EXIT_SUCCESS, flags = 0;

	init(dev, nls, &family, &devid);
	printf("Device `%s` ID: %lld\n", dev, devid);
	printf("Device family ID: %d\n", family);

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	msg = nlmsg_alloc();
	if(!msg) {
		fprintf(stderr, "Failed to allocate netlink message\n");
		nlmsg_free(msg);
		nl_socket_free(nls);
		exit(EXIT_FAILURE);
	}

	cmd = NL80211_CMD_SET_INTERFACE;

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devid);
	NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_MONITOR);


	/* port is 0 - send to kernel */
	genlmsg_put(msg, 0, 0, family, 0, flags, cmd, 0);
	bytes = nl_send_auto(nls, msg);
	if(bytes < 0) {
		fprintf(stderr, "Error send message\n");
		nlmsg_free(msg);
		nl_socket_free(nls);
		exit(EXIT_FAILURE);
	}

	printf("Sent %d bytes\n", bytes);

	bytes = 1;

	while(bytes > 0)
		nl_recvmsgs(nls, cb);

	goto clean_out;

nla_put_failure:
	fprintf(stderr, "Failed to build message\n");
	rcode = EXIT_FAILURE;

clean_out:
	nlmsg_free(msg);
	nl_socket_free(nls);
	exit(rcode);
}
