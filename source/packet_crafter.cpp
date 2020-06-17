#include "packet_crafter.h"

using namespace Tins;

void PacketCrafter::craft_sip_packet(std::string& data)
{
    //run string line by line
    
    //create Sip object 
    Sip packet(...);

    //check for header mandatory fields 
    if(!sip.check_header())
        return;

    //store object in vector
    packets_.push_back(sip);
}


//send dummy packet TODO replace dummy packet with SIP 
void PacketCrafter::send_packets()
{
    NetworkInterface iface = NetworkInterface::default_interface();
    
    NetworkInterface::Info info = iface.addresses();

    //run loop on all packets
    for(Sip& packet : packets_)
    {

    //set raw pdu payload to string in constructor 

    //create ethernet packet 
    EthernetII eth = EthernetII("77:22:33:11:ad:ad", info.hw_addr) / 
                 IP("192.168.0.1", info.ip_addr) /
                 TCP(13, 15) /
                 RawPDU(sip_string);

    PacketSender sender;

    sender.send(eth,iface);
    }
}

PacketCrafter::PacketCrafter(){};
