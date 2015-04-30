#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "constants.h"

int g_fd;
int main(int argc, char *argv[]) {
	int ret = 0;
	g_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
	if (0 > g_fd) {
		perror("create socket error");
		return -1;
	}

	// 设置源地址
	struct sockaddr_nl src_addr;
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pad = 0;
	src_addr.nl_pid = getpid();
	src_addr.nl_groups = 0;
	ret = bind(g_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
	if (-1 == ret) {
		perror("bind");
		return -1;
	}

	// 设置目的地址
	struct sockaddr_nl dst_addr;

	// 设置消息头
	struct nlmsghdr *nlhdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

	struct iovec iov;

	struct msghdr msg;

	while (1) {
		dst_addr.nl_family = AF_NETLINK;
		dst_addr.nl_pad = 0;
		dst_addr.nl_pid = 0;
		dst_addr.nl_groups = 0;

		nlhdr->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
		nlhdr->nlmsg_pid = getpid();
		nlhdr->nlmsg_flags = 0;
		nlhdr->nlmsg_type = 0;

		iov.iov_base = (void *)nlhdr;
		iov.iov_len = nlhdr->nlmsg_len;

		memset(&msg, 0, sizeof(msg));
		msg.msg_name = (void *)&dst_addr;
		msg.msg_namelen = sizeof(dst_addr);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;


		strcpy(NLMSG_DATA(nlhdr), "test");

		ret = sendmsg(g_fd, &msg, 0);
		if (-1 == ret) {
			perror("sendmsg");
			return -1;
		} else {
			printf("sendmsg: [%s]\n", NLMSG_DATA(nlhdr));
		}


		memset(nlhdr, 0, NLMSG_SPACE(MAX_PAYLOAD));
		ret = recvmsg(g_fd, &msg, 0);
		if (-1 == ret) {
			perror("recvmsg");
			return -1;
		} else {
			printf("recvmsg: [%s]\n", NLMSG_DATA(nlhdr));
		}

	}

	return 0;
}
