#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>


#define USER_PORT 100
struct sock *nlsk = NULL;
extern struct net init_net;

#define NETLINK_USR_PROTOCOL 30
int send_usrmsg(char *pbuf, uint16_t len)
{
    int ret;
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;

    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if (!nl_skb) 
    {
        printk("netlink alloc failure\r\n");
        return -1;
    }
    nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_USR_PROTOCOL, len, 0);
    if (nlh == NULL) {
        printk("nlmsg_put failed\r\n");
        nlmsg_free(nl_skb);
        return -1;
    }
    memcpy(nlmsg_data(nlh), pbuf, len);
    ret = netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);
    return ret;
}

void netlink_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    char *kernel_msg = "kernel message"; 
    char *usr_msg = NULL;
  
    if (skb->len >= nlmsg_total_size(0)) {
        nlh = nlmsg_hdr(skb);
        usr_msg = NLMSG_DATA(nlh);
        if (usr_msg) {
            printk("message from user: %s\r\n", usr_msg);
            send_usrmsg(kernel_msg, strlen(kernel_msg));
        }
    }
}

struct netlink_kernel_cfg netlink_cfg = {
    .input = netlink_recv_msg,
};

int netlink_demo_init(void)
{
    nlsk = (struct sock*)netlink_kernel_create(&init_net, NETLINK_USR_PROTOCOL, &netlink_cfg);
    if (nlsk == NULL) {
        return -1;
    }
    printk("%s\r\n", __FUNCTION__);
    return 0;
}

void netlink_demo_exit(void)
{
    if (nlsk) {
        netlink_kernel_release(nlsk);
        nlsk = NULL;
    }
    printk("%s\r\n", __FUNCTION__);
}

module_init(netlink_demo_init);
module_exit(netlink_demo_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("WesleyWang");
MODULE_DESCRIPTION("netlink demo");