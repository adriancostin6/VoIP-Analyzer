/*
 * @Author: yu.tongqing 
 * @Date: 2019-01-13 12:04:32 
 * @Last Modified by: yu.tongqing
 * @Last Modified time: 2019-01-13 13:35:25
 */

#ifndef _NET_HEADER_H_
#define _NET_HEADER_H_

#include <sys/types.h>
typedef struct eth_header
{
    u_char dst[6];
    u_char src[6];
    u_short type;
} eth_header_t;

typedef struct vlan_header
{
	
	u_short id : 12;
	u_short dei : 1;
	u_short priority : 3;
	u_short type;
} vlan_header_t;

typedef struct ip_address
{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
}ip_address_t;
 
typedef struct ip_header
{
    u_char ver_ihl;
    u_char tos;
    u_short tlen;
    u_short identification;
    u_short flags_fo;
    u_char ttl;
    u_char proto;
    u_short crc;
    ip_address saddr;
    ip_address daddr;
}ip_header_t;
 
typedef struct tcp_header
{
    u_short sourport;
    u_short destport;
    unsigned int sequnum;
    unsigned int acknowledgenum;
    u_short headerlenandflag;
    u_short windowsize;
    u_short checksum;
    u_short urgentpointer;
}tcp_header_t;
 
typedef struct udp_header
{
    u_short sourport;
    u_short destport;
    u_short length;
    u_short checksum;
}udp_header_t;

typedef struct rtp_header
{
    u_char cc : 4;
    u_char x  : 1;
    u_char p  : 1;
    u_char v  : 1;
    u_char pt : 7;
    u_char m  : 1;
    u_int16_t seq;
    u_int32_t timestamp;
    u_int32_t ssrc;
    u_int32_t csrc[];
}rtp_header_t;

#define TYPE_VLAN 129
#define TYPE_IPV4 8
#define TYPE_UDP 17

#endif