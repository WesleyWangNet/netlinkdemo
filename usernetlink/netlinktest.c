#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>



#define MSG_LEN  128
#define MAX_PAYLOAD 1024


typedef struct user_msg_info
{
    struct nlmsghdr hdr;
    char msg[MSG_LEN];
}user_msg_info;

#define USER_PORT 100
#define NETLINK_USR_PROTOCOL 30  //self defined
int main(int argc, char **argv)
{
    int skfd;
    int ret;

    user_msg_info uinfo;
    socklen_t len;
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl srcaddr;
    struct sockaddr_nl dstaddr;
    char *netmsg = "netlink from userspace";
    skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USR_PROTOCOL);
    if(skfd == -1) {
        perror("create socket error\r\n");
        return 0;
    }
    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.nl_family = AF_NETLINK;
    srcaddr.nl_pid = USER_PORT;
    srcaddr.nl_groups = 0;
    if(bind(skfd, (struct sockaddr*)&srcaddr, sizeof(srcaddr)) != 0) {
        perror("bind error\r\n");
        close(skfd);
        return -1;
    }
    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.nl_family = AF_NETLINK;
    dstaddr.nl_pid = USER_PORT;
    dstaddr.nl_groups = 0;


    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, sizeof(struct nlmsghdr));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = 0;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = srcaddr.nl_pid;
    memcpy(NLMSG_DATA(nlh), netmsg, strlen(netmsg));
    ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dstaddr, sizeof(struct sockaddr_nl));
    if (!ret) {
        perror("send to error\r\n");
        close(skfd);
        exit(1);
    }
    printf("send: %s\r\n", netmsg);

    memset(&uinfo, 0, sizeof(user_msg_info));
    len = sizeof(struct sockaddr_nl);
    ret = recvfrom(skfd, &uinfo, sizeof(user_msg_info), 0, (struct sockaddr *)&dstaddr, &len);
        if (!ret) {
        perror("recv to error\r\n");
        close(skfd);
        exit(1);
    }
    printf("recv: %s\r\n", uinfo.msg);   
    
    close(skfd);
    skfd = -1;
    free(nlh);
    nlh = NULL;
    
    return 0;
}