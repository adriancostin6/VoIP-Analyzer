#include <iostream>
#include <map>
#include <thread>
#include <chrono>
#include <fstream>
#include <iterator>
#include <string>

#include "capture.h"
//#include "rtp_decoder/decode.h"

//#include "sip.h"
#include "packet_crafter.h"

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

    PacketCrafter p_craft;
    p_craft.craft_sip_packet("input", true);
    p_craft.send_packets();

    try{
//        std::ifstream ifs("input");
//        std::string str(std::istreambuf_iterator<char>{ifs}, {});
//
//        Sip sip(str);
//        sip.print();
//        sip.check_packet("input");
//
//        Tins::Sniffer sniffer(iface.name(),config);
//
//        //added thread for async callback loop stop on keypress
//        //while capturing all packets 
//        Capture cap(Capture::IS_OTHER, "all");
//        std::thread capture_all(&Capture::run_sniffer, &cap, std::ref(sniffer));
//        //cap.run(sniffer);
//
//        //wait for keypress to stop capture
//        std::cin.get();
//
//        cap.loop_stop = true;
//
//        capture_all.join();
//
//        // parse the pcap file and extract the sip data
//        Capture capture_sip(Capture::IS_SIP,"sip");
//        config.set_filter("port 5060");
//        Tins::FileSniffer fsniffer_sip("../temp/all.pcap",config);
//        capture_sip.run_file_sniffer(fsniffer_sip);
//        //write each packet to output directory
//        std::string op_name = "../outputs/packet_";
//        capture_sip.print(op_name);
//
//        //parse pcap file and extract the rtp data
//        Capture capture_rtp(Capture::IS_RTP,"rtp");
//        config.set_filter("udp[1] & 1 != 1 && udp[3] & 1 != 1 && udp[8] & 0x80 == 0x80 && length < 250");
//        Tins::FileSniffer fsniffer_rtp("../temp/all.pcap",config);
//        capture_rtp.run_file_sniffer(fsniffer_rtp);
//
//
//
//        //get ports and ips needed for RTP decoding 
//        auto ports_and_ips = capture_rtp.getPorts();
//        std::string server_ip = "192.168.1.8";
//
//        //keep this for later logging 
//        //for(auto& key_pair : ports_and_ips)
//        //{
//        //    auto ip_pair = key_pair.first;
//        //    auto port_pair = key_pair.second;
//
//        //    std::cout << "Source IP: "<< ip_pair.first << " Destination IP: "<< ip_pair.second<< "\n";
//        //    std::cout << "Source Port: "<< port_pair.first << " Destination Port: "<< port_pair.second<< "\n";
//        //}
//
//        for(auto it=ports_and_ips.begin(); it!=ports_and_ips.end();)
//        {
//            if(it->first.first == server_ip)
//                ports_and_ips.erase(it++);
//            else
//                ++it; 
//        }
//
//        std::cout << "Pairs you were looking for \n";
//        for(auto& key_pair : ports_and_ips)
//        {
//            auto ip_pair = key_pair.first;
//            auto port_pair = key_pair.second;
//
//            std::cout << "Source IP: "<< ip_pair.first << " Destination IP: "<< ip_pair.second<< "\n";
//            std::cout << "Source Port: "<< port_pair.first << " Destination Port: "<< port_pair.second<< "\n";
//        }
//        int i = 1;
//        for(auto& key_pair : ports_and_ips)
//        {
//            std::string out_filename = "Speaker_" + std::to_string(i);
//            auto ip_pair = key_pair.first;
//            auto port_pair = key_pair.second;
//            
//            decode("../temp/rtp.pcap", out_filename, ip_pair.first, ip_pair.second, port_pair.first, port_pair.second);
//            i++;
//        }
    }catch(const std::string& sip_ex){
        std::cout << "Error crafting SIP packet: ";
        std::cout << sip_ex << "\n";
    }
    catch (std::exception& ex){
        std::cerr << "Error: " << ex.what() << "\n";
    }
}


