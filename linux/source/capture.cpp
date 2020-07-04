#include "capture.h"

#include <fstream>
#include <memory>
#include <functional>

#include "sip.h"

using namespace Tins;

//Constructor for Capture object
//  parameters:
//      - c : enum CaptureType
//      - filename : const string&
//  returns: Capture object
Capture::Capture(CaptureType c, const std::string& filename)
{
    //set boolean member variables depending on capture type
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

//Callback function for sniff_loop
//  parameters:
//      - pdu : PDU&
//  use:
//      - split captured packet data into protocols and analyze them
//      - create objects of type Sip and Rtp from the PDU and store them 
//  returns: bool (false breaks the loop) 
bool Capture::callback(const PDU& pdu)
{
    //quit capture loop if key on main thread is pressed
    if(loop_stop)
        return false;

    //store the PDU in a Packet object for writing it to a PCAP file
    Packet packet = pdu;

    //get IP, UDP and RAW payload data
    const IP& ip = pdu.rfind_pdu<IP>();
    const UDP& udp = pdu.rfind_pdu<UDP>();
    const RawPDU& raw = udp.rfind_pdu<RawPDU>();

    if(capture_sip)
    {
        //print the source and destination ip and port values
        //important for informing the user about the address of the port
        std::cout << ip.src_addr() << ":" << udp.sport() << " -> "
            << ip.dst_addr()<< " : " <<udp.dport() << "\n";

        //create Sip object from RawPDU payload
        const Sip& sip= raw.to<Sip>();

        //skip empty packets
        if(sip.get_header_order()[0] == "\r")
            return true;

        //write sip packet to a temp file in case we want to look at it in wireshark
        p_writer->write(packet);

        //store packets in vector
        packets_.push_back(sip);
    }
    else
    {
        //print the source and destination ip and port values
        //turned off for cleaner output
        //std::cout << ip.src_addr() << ":" << udp.sport() << " -> "
        //    << ip.dst_addr()<< " : " <<udp.dport() << "\n";

        if(capture_rtp)
        {
            //construct Rtp object from RawPDU payload
            const Rtp& rtp_packet = raw.to<Rtp>();

            //store the packet in a vector
            rtp_packets_.push_back(rtp_packet);

            //store the source and destination ip and port in a map
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

        //write packet to PCAP file
        p_writer->write(packet);
    }
    return true;
}

//Wrapper functions for the sniff_loop for both the run_sniffer and the 
//file sniffer
//Binds the callback function to the sniff loop and captures packets as
//long as the callback returns true
void Capture::run_sniffer(Sniffer& sniffer)
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
void Capture::run_file_sniffer(Tins::FileSniffer& fsniffer) 
{
    fsniffer.sniff_loop(std::bind(
                &Capture::callback,
                this,
                std::placeholders::_1
                )
            );
}


//Printing functions for the captured Sip packets
//Support for both printing to the console and to files
void Capture::print(std::string& path) const
{
    //get packet size
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

//getter functions for member variables
std::map<std::pair<std::string,std::string>,
    std::pair<std::string,std::string>> Capture::get_ports()
{
    return rtp_ips_and_ports;
}
std::vector<Rtp> Capture::get_rtp_packets()
{
    return rtp_packets_;
}
std::vector<Sip> Capture::get_sip_packets()
{
    return packets_;
}
