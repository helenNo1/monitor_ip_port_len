#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/inet.h>

static struct nf_hook_ops nfho;

const char *get_protocol_name(unsigned char protocol) {
    switch (protocol) {
        case IPPROTO_TCP:
            return "TCP";
        case IPPROTO_UDP:
            return "UDP";
        case IPPROTO_ICMP:
            return "ICMP";
        // 其他协议的处理
        default:
            return "Unknown";
    }
}

// 定义一个钩子函数，用于处理传入的网络数据包
static unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
  struct iphdr *ip_header;
  struct tcphdr *tcp_header;
  struct udphdr *udp_header;

    // 从数据包中获取 IP 头部信息
  ip_header = ip_hdr(skb);

// 检查目标地址是否为私有网络地址
    if ( (ntohl(ip_header->daddr) & 0xff000000) == 0x0a000000 ||           // 10.0.0.0/8
         (ntohl(ip_header->daddr) & 0xfff00000) == 0xac100000 ||           // 172.16.0.0/12
         (ntohl(ip_header->daddr) & 0xffff0000) == 0xc0a80000) {           // 192.168.0.0/16
        return NF_ACCEPT;
    }

  if(ip_header->protocol == IPPROTO_TCP){
    tcp_header = tcp_hdr(skb);
    printk(KERN_INFO "Source IP: %pI4, Destination IP: %pI4 DstPort:%d Protocol:%s len: %d\n", &ip_header->saddr, &ip_header->daddr, ntohs(tcp_header->dest), get_protocol_name(ip_header->protocol), ip_header->tot_len);

   
  }else if(ip_header->protocol == IPPROTO_UDP) {
    udp_header = udp_hdr(skb);
    printk(KERN_INFO "Source IP: %pI4, Destination IP: %pI4 DstPort:%d Protocol:%s len: %d\n", &ip_header->saddr, &ip_header->daddr, ntohs(udp_header->dest), get_protocol_name(ip_header->protocol), ip_header->tot_len);
   }


  
    // 返回 NF_ACCEPT 表示接受数据包
  return NF_ACCEPT;
}

// 模块初始化函数
static int __init init_nf_module(void)
{
    // 初始化钩子函数
    nfho.hook = hook_func;                   // 设置钩子函数
    nfho.hooknum = NF_INET_POST_ROUTING;      // 设置钩子位置，在数据包路由之前
    //nfho.hooknum = NF_INET_PRE_ROUTING;      // 设置钩子位置，在数据包路由之前
    nfho.pf = PF_INET;                       // 设置钩子协议族为 IPv4
    nfho.priority = NF_IP_PRI_FIRST;         // 设置钩子优先级

    // 注册钩子函数
    nf_register_net_hook(&init_net, &nfho);

    printk(KERN_INFO "Netfilter module loaded\n");

    return 0;
}

// 模块退出函数
static void __exit cleanup_nf_module(void)
{
    // 注销钩子函数
    nf_unregister_net_hook(&init_net, &nfho);

    printk(KERN_INFO "Netfilter module unloaded\n");
}

module_init(init_nf_module);
module_exit(cleanup_nf_module);

MODULE_LICENSE("GPL");

