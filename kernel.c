#include <linux/module.h>
#include <linux/netlink.h>
#include <net/sock.h>

#include "constants.h"

struct sock *g_nl_sk = NULL;
int user_space_pid = -1;

int send_msg_to_user_space(char *message) {
	if (NULL == g_nl_sk || -1 == user_space_pid) {
		printk("not connect to user space progress, please wait...data: [%s]\n", message);
		return -1;
	}

	int msg_size = strlen(message);
	struct sk_buff *out_skb = nlmsg_new(msg_size, GFP_ATOMIC);
	if (NULL == out_skb) {
		printk(KERN_ERR "alloc_skb error for out_skb\n");
		return -1;
	}

	struct nlmsghdr *nlhdr = nlmsg_put(out_skb, 0, 0, NLMSG_DONE, msg_size, 0);
	if (NULL == nlhdr) {
		/* nlmsg_free(out_skb); */
		printk(KERN_ERR "nlmsg_put error\n");
		return -1;
	}

	NETLINK_CB(out_skb).pid = 0;
	NETLINK_CB(out_skb).dst_group = 0;

	strncpy(nlmsg_data(nlhdr), message, msg_size);
	int ret = nlmsg_unicast(g_nl_sk, out_skb, user_space_pid);
	printk("nlmsg_unicast: data = [%s], ret = [%d]\n", (char *)nlmsg_data(nlhdr), ret);
	/* nlmsg_free(out_skb); */
	return 0;
}

void nl_data_ready(struct sk_buff *skb) {
	printk("before get data\n");
	struct nlmsghdr *nlh = nlmsg_hdr(skb);
	user_space_pid = nlh->nlmsg_pid;

	char *str = (char *)nlmsg_data(nlh);
	printk("received from: [%d], data: [%s]\n", user_space_pid, str);

	send_msg_to_user_space(str);
	printk("send message to pid: [%d]\n", user_space_pid);
}

int __init netlink_test_init(void) {
	g_nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, 0, nl_data_ready, NULL, THIS_MODULE);
	if (NULL == g_nl_sk) {
		printk("create sk error\n");
		return -1;
	}

	printk("init netlink_test\n");
	return 0;
}

void __exit netlink_test_exit(void) {
	if (g_nl_sk) {
		netlink_kernel_release(g_nl_sk);
	}
}

module_init(netlink_test_init);
module_exit(netlink_test_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("http://www.logevery.com");
