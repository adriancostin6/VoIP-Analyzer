#include "capture.h"
#include "sip.h"

using namespace Tins;

Capture::Capture(){};

bool Capture::callback(const PDU& pdu)
{    
    const IP& ip = pdu.rfind_pdu<IP>();
    const UDP& udp = pdu.rfind_pdu<UDP>();

    std::cout << ip.src_addr() << ":" << udp.sport() << " -> "
              << ip.dst_addr()<< " : " <<udp.dport() << "\n"; 

    const RawPDU& raw = udp.rfind_pdu<RawPDU>();

    const Sip& sip= raw.to<Sip>();

    packets_.push_back(sip);

    if(packets_.size() == 3)
        return false;

    return true;
}

void Capture::run(Sniffer& sniffer)
{
    sniffer.sniff_loop(std::bind(
                &Capture::callback,
                this,
                std::placeholders::_1
                )
            );
}

void Capture::print() const
{
    for(auto const& pack : packets_)
         pack.print();
}
