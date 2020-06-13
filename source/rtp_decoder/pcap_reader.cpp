/*
 * @Author: yu.tongqing 
 * @Date: 2019-01-13 10:09:27 
 * @Last Modified by: yu.tongqing
 * @Last Modified time: 2019-01-13 13:57:48
 */

#include "pcap_reader.h"
#define JITTER_BUFFER_SIZE 50

pcap_reader::pcap_reader(std::string& filename):m_buffer(JITTER_BUFFER_SIZE)
{
    char error[100];
    m_pcap_handler = pcap_open_offline(filename.c_str(), error);
    if(!m_pcap_handler)
    {
        printf("open file error: %s\n", error);
    }
}

pcap_reader::~pcap_reader()
{
    if(m_pcap_handler)
    {
        pcap_close(m_pcap_handler);
    }
}

std::shared_ptr<rtp_packet> pcap_reader::get_next_rtp(const char* src_ip, const char* dst_ip, const u_short src_port, const u_short dst_port)
{
    const unsigned char *p_packet_content = NULL;
    struct pcap_pkthdr protocol_header;
    bool clear = false;

    std::shared_ptr<rtp_packet> result;
    while((result = m_buffer.get_packet(clear)) == NULL)
    {
        //get next packet
        while(p_packet_content = pcap_next(m_pcap_handler, &protocol_header))
        {
            if(protocol_header.caplen > sizeof(eth_header_t))
            {
                const unsigned char* p = p_packet_content;
                u_short ip_type = 0;

                //process ethernet header
                const eth_header_t* eth_h = (const eth_header_t*) p;
                p += sizeof(eth_header_t);
                //printf("eth src %2x:%2x:%2x:%2x:%2x:%2x, dst %2x:%2x:%2x:%2x:%2x:%2x, type %u\n",
                //eth_h->src[0], eth_h->src[1], eth_h->src[2], eth_h->src[3], eth_h->src[4], eth_h->src[5],
                //eth_h->dst[0], eth_h->dst[1], eth_h->dst[2], eth_h->dst[3], eth_h->dst[4], eth_h->dst[5],
                //eth_h->type);
                
                //process vlan
                if(eth_h->type == TYPE_VLAN)
                {
                    const vlan_header_t* vlan_h = (const vlan_header_t*) p;
                    p += sizeof(vlan_header_t);
                    ip_type = vlan_h->type;
                }else
                {
                    ip_type = eth_h->type;
                }

                //process ipv4 header
                if(ip_type != TYPE_IPV4)
                {
                    continue;
                }
                const ip_header_t* ip_h = (const ip_header_t*) p;
                p += sizeof(ip_header_t);
                char src[100];
                char dst[100];
                snprintf(src, sizeof(src), "%u.%u.%u.%u", ip_h->saddr.byte1, ip_h->saddr.byte2, ip_h->saddr.byte3, ip_h->saddr.byte4);
                snprintf(dst, sizeof(dst), "%u.%u.%u.%u", ip_h->daddr.byte1, ip_h->daddr.byte2, ip_h->daddr.byte3, ip_h->daddr.byte4);
                //printf("ip src %s, dst %s, type: %u\n", src, dst, ip_h->proto);
                if(strcmp(src, src_ip) || strcmp(dst, dst_ip))
                {
                    continue;
                }

                //process udp header
                if(ip_h->proto != TYPE_UDP)
                {
                    continue;
                }
                const udp_header_t* udp_h = (const udp_header_t*) p;
                p += sizeof(udp_header_t);
                if(src_port != ntohs(udp_h->sourport) || dst_port != ntohs(udp_h->destport))
                {
                    continue;
                }
                
                //process rtp
                const rtp_header_t* rtp_h = (const rtp_header_t*) p;
                p += sizeof(rtp_header_t);

                //process rtp extension
                u_int16_t ext_len = 0;
                if(rtp_h->x)
                {
                    p += 2;
                    ext_len = ntohs(*(u_int16_t*)p);
                    p += sizeof(ext_len) + ext_len*4;
                    ext_len += ext_len * 4 + 2;
                }

                std::shared_ptr<rtp_packet> rtp_packet_ptr(new rtp_packet);
                rtp_packet_ptr->header = *rtp_h;
                rtp_packet_ptr->header.seq = ntohs(rtp_h->seq);
                rtp_packet_ptr->header.timestamp = ntohl(rtp_h->timestamp);
                rtp_packet_ptr->data = std::string((const char*)p, (size_t)ntohs(udp_h->length) - sizeof(udp_header_t) - sizeof(rtp_header_t) - ext_len);

                m_buffer.put_packet(rtp_packet_ptr);
                break;
            }
        }
        if(!p_packet_content)
        {
            if(clear)
            {
                break;
            }
            clear = true;
        }
    }
    return result;
}

jitter_buffer::jitter_buffer(size_t buf_size): m_size(buf_size), m_last_seq(-1)
{
}

jitter_buffer::~jitter_buffer()
{
    m_list.clear();
}

void jitter_buffer::put_packet(std::shared_ptr<rtp_packet> packet)
{
    if(m_list.size() == 0)
    {
        m_list.push_back(packet);
        return;
    }
    auto iter = m_list.begin();
    for(; iter != m_list.end();)
    {
        if(packet->header.seq > (*iter)->header.seq)
        {
            ++iter;
        }else if(packet->header.seq < (*iter)->header.seq)
        {
            m_list.insert(++iter, packet);
            return;
        }else
        {
            return;
        }
    }
    m_list.push_back(packet);
    return;
}

std::shared_ptr<rtp_packet> jitter_buffer::get_packet(bool clear)
{
    auto iter = m_list.begin();
    if(iter == m_list.end())
    {
        return NULL;
    }
    if(clear || m_last_seq == -1 || m_list.size() >= m_size || m_last_seq+1 == (*iter)->header.seq)
    {
        auto result = *iter;
        m_list.pop_front();
        return result;
    }
    return NULL;
}