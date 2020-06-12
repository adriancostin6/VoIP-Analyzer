#include <iostream>
#include <map>

#include "capture.h"



int main()
{
    Tins::NetworkInterface iface = Tins::NetworkInterface::default_interface();
    Tins::NetworkInterface::Info info = iface.addresses();

    std::cout<< "Network interface: "<< iface.name()
        <<" IP ADDR: "<< info.ip_addr<< "\n";

    Tins::SnifferConfiguration config;
    config.set_promisc_mode(true);
    //"udp[1] & 1 != 1 && udp[3] & 1 != 1 && udp[8] & 0x80 == 0x80 && length < 250" filter for RTP
    //"port 5060" filter for SIP
    config.set_filter("udp[1] & 1 != 1 && udp[3] & 1 != 1 && udp[8] & 0x80 == 0x80 && length < 250");
    config.set_immediate_mode(true);

    try{

        //Tins::Sniffer sniffer(iface.name(),config);
        Tins::FileSniffer fsniffer("../temp/all.pcap",config);

        Capture cap(Capture::IS_RTP, "rtp");
        cap.run(fsniffer);

        //print files read from file sniffer
 //       std::string op_name = "../outputs/packet_";
//        cap.print(op_name);
        auto ports_and_ips = cap.getPorts();
        std::string server_ip = "192.168.1.8";
        for(auto& key_pair : ports_and_ips)
        {
            auto ip_pair = key_pair.first;
            auto port_pair = key_pair.second;

            std::cout << "Source IP: "<< ip_pair.first << " Destination IP: "<< ip_pair.second<< "\n";
            std::cout << "Source Port: "<< port_pair.first << " Destination Port: "<< port_pair.second<< "\n";
        }

        for(auto it=ports_and_ips.begin(); it!=ports_and_ips.end();)
        {
            if(it->first.first == server_ip)
               ports_and_ips.erase(it++);
            else
               ++it; 
        }

        std::cout << "Pairs you were looking for \n";
        for(auto& key_pair : ports_and_ips)
        {
            auto ip_pair = key_pair.first;
            auto port_pair = key_pair.second;

            std::cout << "Source IP: "<< ip_pair.first << " Destination IP: "<< ip_pair.second<< "\n";
            std::cout << "Source Port: "<< port_pair.first << " Destination Port: "<< port_pair.second<< "\n";
        }
    }catch (std::exception& ex){
        std::cerr << "Error: " << ex.what() << "\n";
    }
}


