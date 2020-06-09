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
    
    if(sip.getHeader().size() != 0)
        packets_.push_back(sip);

    if(packets_.size() == 10)
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
void Capture::run(Tins::FileSniffer& fsniffer)
{
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

