#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/genl/genl.h>
#include <netinet/if_ether.h>

#include <pcap.h>

#include "nl80211.h"

static void init(const char *dev, struct nl_sock *nls, int *family, signed long long *devid) {
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
	printf("Ack recv\n");
	*((int*)arg)  = 0;
	return NL_STOP;
}

static int finish_cb(struct nl_msg *msg, void *arg) {
	printf("Finish recv\n");
	*((int*)arg)  = 0;
	return NL_SKIP;
}

static int error_cb(struct nl_msg *msg, struct nlmsgerr *err, void *arg) {
	printf("Error on recv: %d\n", err->error);
	*((int*)arg)  = err->error;
	return NL_SKIP;
}

static int set_mntr_control_flag(struct nl_msg *msg) {
	struct nl_msg *flags = nlmsg_alloc();

	if(!flags) {
		fprintf(stderr, "Error allocating flags\n");
		nlmsg_free(flags);
		exit(EXIT_FAILURE);
	}

	NLA_PUT_FLAG(flags, NL80211_MNTR_FLAG_CONTROL);
	nla_put_nested(msg, NL80211_ATTR_MNTR_FLAGS, flags);
	return 0;

nla_put_failure:
	return -1;
}

/* TODO:
 * this needs to be researched more deeply
 * to see what net-tools is up to...
 */
static int ifconfig_device_up(const char *dev) {


	char ifcfg[20];
	sprintf(ifcfg, "ifconfig %s up", dev);
	printf("$ %s\n", ifcfg);

	return system(ifcfg);
}

static int ifconfig_device_down(const char *dev) {


	char ifcfg[20];
	sprintf(ifcfg, "ifconfig %s down", dev);
	printf("$ %s\n", ifcfg);

	return system(ifcfg);
}

int main( int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Device unspecified\n");
		exit(EXIT_FAILURE);
	}

	char *dev = argv[1], errbuf[PCAP_ERRBUF_SIZE], ifcmd[20], *ifptr;
	struct nl_msg *msg = NULL;
	struct nl_sock *nls = nl_socket_alloc();
	struct nl_cb *cb = NULL;

	signed long long devid;
	int family, cmd, bytes, r, rcode = EXIT_SUCCESS, flags = 0;

	init(dev, nls, &family, &devid);
	printf("Device ID: %lld\n", devid);
	printf("Netlink family ID: %d\n", family);

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	msg = nlmsg_alloc();
	if(!msg) {
		fprintf(stderr, "Failed to allocate netlink message\n");
		nlmsg_free(msg);
		nl_socket_free(nls);
		exit(EXIT_FAILURE);
	}

	/* port is 0 - send to kernel */
	genlmsg_put(msg, 0, 0, family, 0, flags, NL80211_CMD_SET_INTERFACE, 0);


	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devid);
	NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_MONITOR);


	if(set_mntr_control_flag(msg) < 0)
		goto nla_put_failure;
	
	nl_socket_set_cb(nls, cb); /* Heavens to murgatroyd - this function is undocumented */
/*      ^^^^^^^^^^^^^^^^
	 * After some digging around in libnl these *_{put|set}_cb functions are to do
	 * with reference counting.
	 * 
	 * nl_socket_set_cb():
	 * 
	 * decreases the reference count of the current callback function structure
	 * and increments the reference count of the structure passed in and
	 * assigns it tp the socket structure
	 */

	bytes = nl_send_auto(nls, msg);
	if(bytes < 0) {
		fprintf(stderr, "Error sending message\n");
		nlmsg_free(msg);
		nl_socket_free(nls);
		exit(EXIT_FAILURE);
	}

	printf("Sent %d bytes\n", bytes);

	bytes = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, (nl_recvmsg_err_cb_t)error_cb, (void*)(&bytes));
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, (nl_recvmsg_msg_cb_t)finish_cb, (void*)(&bytes));
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, (nl_recvmsg_msg_cb_t)ack_cb, (void*)(&bytes));
	while(bytes > 0)
		nl_recvmsgs(nls, cb);

	r = ifconfig_device_up(dev);
	if(r == -1)
		fprintf(stderr, "Error in creating child process\n");
	else if(r == EXIT_FAILURE)
		fprintf(stderr, "Child process EXIT_FAILURE\n");
	else if(r == EXIT_SUCCESS)
		fprintf(stdout, "device is up\n", dev);

	goto clean_out;

nla_put_failure:
	fprintf(stderr, "Failed to build message\n");
	rcode = EXIT_FAILURE;

clean_out:
	nlmsg_free(msg);
	nl_socket_free(nls);
	exit(rcode);
}
