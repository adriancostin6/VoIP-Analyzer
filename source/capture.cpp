#include "capture.h"

#include <fstream>
#include <memory>
#include <functional>

#include "sip.h"

using namespace Tins;

Capture::Capture(CaptureType c, const std::string& filename)
{

    if(c == CaptureType::IS_SIP)
        capture_sip = true;
    else
        capture_sip = false;

    if(c== CaptureType::IS_RTP)
        capture_rtp = true;
    else
        capture_rtp = false;

    //for live capture create a pcap file to store the packets
    std::string file_path = "../temp/" + filename + ".pcap";
    p_writer = std::make_unique<PacketWriter>(file_path,DataLinkType<EthernetII>());
};

bool Capture::callback(const PDU& pdu)
{
    Packet packet = pdu;
    const IP& ip = pdu.rfind_pdu<IP>();
    const UDP& udp = pdu.rfind_pdu<UDP>();
    const RawPDU& raw = udp.rfind_pdu<RawPDU>();

    if(capture_sip)
    {
        std::cout << ip.src_addr() << ":" << udp.sport() << " -> "
            << ip.dst_addr()<< " : " <<udp.dport() << "\n";
        const Sip& sip= raw.to<Sip>();


        //skip empty packets
        if(sip.getHeader()[0] == "\r")
            return true;

        //write sip packet to a temp file in case we want to look at it in wireshark
        p_writer->write(packet);

        //if(isFileSniffer)
        //{

        packets_.push_back(sip);
         //   return true;
        //}


        //print packets to output files
        //std::string path = "../outputs/sip/packet_";
        //sip.print(path, pkg_num);
        //pkg_num++;
    }
    else
    {
        std::cout << ip.src_addr() << ":" << udp.sport() << " -> "
            << ip.dst_addr()<< " : " <<udp.dport() << "\n";

        //if it is an rtp packet store its port and ip 
        if(capture_rtp)
        {
            rtp_ips_and_ports.insert(
                    std::make_pair(
                        std::make_pair(
                            ip.src_addr().to_string(),
                            ip.dst_addr().to_string()
                            ),
                        std::make_pair(
                            std::to_string(udp.sport()),
                            std::to_string(udp.dport())
                            )
                        )
                    );
        }

        p_writer->write(packet);
    }

    return true;
}

void Capture::run(Sniffer& sniffer)
{
    //isFileSniffer = false;
    //pkg_num = 1;
    sniffer.sniff_loop(std::bind(
                &Capture::callback,
                this,
                std::placeholders::_1
                )
            );
}
void Capture::run(Tins::FileSniffer& fsniffer)
{
    //isFileSniffer = true;
    fsniffer.sniff_loop(std::bind(
                &Capture::callback,
                this,
                std::placeholders::_1
                )
            );
}


void Capture::print(std::string& path) const
{
    unsigned sz = packets_.size();
    unsigned dif = sz; 
    for(auto const& pack : packets_)
    {
        //each packet will have a separate output file
        //with a name given by "packet_name" + "sz-dif+1"
        //this keeps the output files in ascending order
        if(dif > 0)
        {
            pack.print(path,sz-dif+1);
            dif--;
        }
    }
}

void Capture::print() const
{
    for(auto const& pack: packets_)
        pack.print();
}

std::map<std::pair<std::string,std::string>,
    std::pair<std::string,std::string>> Capture::getPorts()
{
    return rtp_ips_and_ports;
}
