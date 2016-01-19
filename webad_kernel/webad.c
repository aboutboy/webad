#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/in.h>
#include <linux/ctype.h>
#include <linux/decompress/mm.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/netlink.h> 
#include <linux/socket.h> 
#include <linux/types.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netdevice.h>

#include <net/netfilter/nf_log.h>
#include <net/netfilter/nf_nat_helper.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/tcp.h>
#include <net/ip_fib.h>
#include <net/rtnetlink.h>
#include <net/ip_fib.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>


MODULE_LICENSE("GPL");

MODULE_AUTHOR("yj");

unsigned int capture(unsigned int hooknum,
                struct sk_buff *skb,
                const struct net_device *in,
                const struct net_device *out,
                int (*okfn)(struct sk_buff *))
{

    struct iphdr *pIph = NULL;
    struct tcphdr *pTcph = NULL;
    unsigned short tcpSrcPort=0;
    unsigned short tcpDstPort=0; 
	
    if(NULL == skb)
    {
        return NF_ACCEPT;
    }

    pIph = ip_hdr(skb);
    if(NULL == pIph)
    {
        return NF_ACCEPT;
    }
    
    if (0 != skb_linearize(skb))
    {
        printk(" !!! skb_linearize error\n");
        return NF_ACCEPT;
    }

    /* Is protocol TCP */
    if(pIph->protocol != IPPROTO_TCP)
    {
        //printk("it is not TCP\n");
        return NF_ACCEPT;
    }
    
   
    /*port 80 is used for http*/
    pTcph = (struct tcphdr *)((u8 *)pIph + (pIph->ihl<<2));

#if defined(__LITTLE_ENDIAN_BITFIELD)
	tcpSrcPort = ntohs(pTcph->source);
    tcpDstPort = ntohs(pTcph->dest);
    
#else
	tcpSrcPort = pTcph->source;
    tcpDstPort = pTcph->dest;
    
#endif

    if(tcpSrcPort == 80 || tcpDstPort == 80)
    {   
        return NF_QUEUE;
    }
	
    return NF_ACCEPT;
}



static struct nf_hook_ops HttpOps[] =
{

    {
        {NULL,NULL},
        .hook               = (nf_hookfn *)capture,
        .owner              = THIS_MODULE,
        .pf                 = PF_INET,
        .hooknum            = NF_INET_LOCAL_IN,
        .priority           = NF_IP_PRI_FIRST,
    },
    
    {
        {NULL,NULL},
        .hook               = (nf_hookfn *)capture,
        .owner              = THIS_MODULE,
        .pf                 = PF_INET,
        .hooknum            = NF_INET_POST_ROUTING,
        .priority           = NF_IP_PRI_FIRST,
    },
};



static int __init webad_init(void)
{
    int ret;
    printk(KERN_EMERG "!!!!!!!!! webad_init\n");
	
    ret = nf_register_hooks(HttpOps, ARRAY_SIZE(HttpOps));
    if(ret < 0)
    {
        printk("%s\n", "can't modify skb hook!");
        return ret;
    }

    return 0;
}

static void webad_exit(void)
{

    nf_unregister_hooks(HttpOps, ARRAY_SIZE(HttpOps));
    
    printk(KERN_EMERG"!!!!!!!!! webad_exit\n");
}

module_init(webad_init);
module_exit(webad_exit);

