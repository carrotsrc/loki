#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/genl/genl.h>
#include <stdlib.h>

#include "nl80211.h"

/* free netlink stuff */
inline void free_nlmem(struct nl_msg *msg, struct nl_sock *nls) {
	if(msg != NULL)
		nlmsg_free(msg);

	if(nls != NULL)
		nl_socket_free(nls);
}

/* init the netlink socket */
static void init(const char*, struct nl_sock*, int*, signed long long*);

/* callbacks for message recv */
static int ack_cb(struct nl_msg*, void*);
static int finish_cb(struct nl_msg*, void*);
static int error_cb(struct nl_msg*, struct nlmsgerr*, void*);

/* set the monitor control flag */
static int set_mntr_control_flag(struct nl_msg*, int);

unsigned int set_monitor_mode(const char *dev) {
	signed long long devid;
	int family, cmd, bytes, flags = 0;

	struct nl_msg *msg = NULL;
	struct nl_cb *cb = NULL;
	struct nl_sock *nls = nl_socket_alloc();

	if(nls == NULL) {
		fprintf(stderr, "Netlink: NULL netlink socket\n");
		exit(1);
	}

	init(dev, nls, &family, &devid);
	printf("FD: %d\n", nl_socket_get_fd(nls));
	printf("Device ID: %lld\n", devid);
	printf("Netlink family ID: %d\n", family);

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	msg = nlmsg_alloc();
	if(!msg) {
		fprintf(stderr, "Failed to allocate netlink message\n");
		nlmsg_free(msg);
		nl_socket_free(nls);
		return 1;
	}

	/* port is 0 - send to kernel */
	genlmsg_put(msg, 0, 0, family, 0, flags, NL80211_CMD_SET_INTERFACE, 0);


	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devid);
	NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_MONITOR);


	if(set_mntr_control_flag(msg, NL80211_MNTR_FLAG_CONTROL) < 0)
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
		free_nlmem(msg, nls);
		return 1;
	}

	bytes = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, (nl_recvmsg_err_cb_t)error_cb, (void*)(&bytes));
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, (nl_recvmsg_msg_cb_t)finish_cb, (void*)(&bytes));
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, (nl_recvmsg_msg_cb_t)ack_cb, (void*)(&bytes));
	while(bytes > 0)
		nl_recvmsgs(nls, cb);

	free_nlmem(msg, nls);
	return 0;

nla_put_failure: /* labels in your macros, libnl? I shake a tiny fist at you*/
	fprintf(stderr, "Failed to build message\n");
	free_nlmem(msg, nls);
	return 1;
}

static void init(const char *dev, struct nl_sock *nls, int *family, signed long long *devid) {
	if(!nls) {
		fprintf(stderr, "Netlink: Failed to allocate socket\n");
		exit(EXIT_FAILURE);
	}

	int r = nl_socket_set_buffer_size(nls, 8192, 8192);
	/* this creates a local socket + binds to GENERIC NETLINK*/
	if(genl_connect(nls)) {
		fprintf(stderr, "Netlink: Failed to create socket fd\n");
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
	printf("Ack OK\n");
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

static int set_mntr_control_flag(struct nl_msg *msg, int flag) {
	struct nl_msg *flags = nlmsg_alloc();

	if(!flags) {
		fprintf(stderr, "Error allocating flags\n");
		nlmsg_free(flags);
		exit(EXIT_FAILURE);
	}

	NLA_PUT_FLAG(flags, flag);
	nla_put_nested(msg, NL80211_ATTR_MNTR_FLAGS, flags);
	return 0;

nla_put_failure:
	nlmsg_free(flags);
	return -1;
}
