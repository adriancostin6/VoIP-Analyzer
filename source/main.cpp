#include <iostream>
#include <map>
#include <thread>
#include <chrono>

#include "capture.h"
#include "rtp_decoder/decode.h"

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
    // config.set_filter("udp[1] & 1 != 1 && udp[3] & 1 != 1 && udp[8] & 0x80 == 0x80 && length < 250");
    config.set_immediate_mode(true);

    try{

        Tins::Sniffer sniffer(iface.name(),config);

        //added thread for async callback loop stop on keypress
        Capture cap(Capture::IS_OTHER, "all");
        std::thread capture1(&Capture::run_sniffer, &cap, std::ref(sniffer));
        //cap.run(sniffer);

        std::cin.get();

        cap.loop_stop = true;

        capture1.join();
        

        Capture cap1(Capture::IS_SIP,"sip");
        config.set_filter("port 5060");
        Tins::FileSniffer fsniffer("../temp/all.pcap",config);
        cap1.run_file_sniffer(fsniffer);

        Capture cap2(Capture::IS_RTP,"rtp");
        config.set_filter("udp[1] & 1 != 1 && udp[3] & 1 != 1 && udp[8] & 0x80 == 0x80 && length < 250");
        Tins::FileSniffer fsniffer1("../temp/all.pcap",config);
        cap2.run_file_sniffer(fsniffer1);


        //print sip files read from file sniffer
        //std::string op_name = "../outputs/packet_";
        //cap.print(op_name);

        //get ports and ips needed for RTP decoding 
        auto ports_and_ips = cap2.getPorts();
        std::string server_ip = "192.168.1.8";

        //keep this for later logging 
        //for(auto& key_pair : ports_and_ips)
        //{
        //    auto ip_pair = key_pair.first;
        //    auto port_pair = key_pair.second;

        //    std::cout << "Source IP: "<< ip_pair.first << " Destination IP: "<< ip_pair.second<< "\n";
        //    std::cout << "Source Port: "<< port_pair.first << " Destination Port: "<< port_pair.second<< "\n";
        //}

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
        int i = 1;
        for(auto& key_pair : ports_and_ips)
        {
            std::string out_filename = "Speaker" + i;
            auto ip_pair = key_pair.first;
            auto port_pair = key_pair.second;
            
            decode("../temp/rtp.pcap",out_filename,ip_pair.first,ip_pair.second,port_pair.first,port_pair.second);
            i++;
        }
    }catch (std::exception& ex){
        std::cerr << "Error: " << ex.what() << "\n";
    }
}


