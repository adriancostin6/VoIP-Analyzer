#ifndef CAPTURE_H
#define CAPTURE_H
#include <vector>
#include <list>
#include <iostream>
#include <memory>
#include <string>
#include <map>

#include <tins/tins.h>

#include "sip.h"
#include "rtp/rtp.h"

//Capture class
//Used for capturing and storing SIP and RTP packets, as well as RTP ports
//used in the decoder.
//Parameters:
//  - CaptureType : enum (defines three types of packets that can be captured)
//  - loop_stop : bool (stops the sniff_loop prematurely when true)
//  - p_writer : PacketWriter (used for writing PCAP files)
//  - packets_ : vector<Sip> (used for storing captured SIP packets)
//  - rtp_packets_ : vector<Rtp> (used for storing RTP packets)
//  - rtp_ips_and_ports : map (used for storing ip and port pairs)
//  - capture_sip, capture_rtp : bool (used in callback function packet capture)
//Operations:
//  - Capture() : constructor
//  - run_sniffer() : wrapper for sniff_loop
//  - run_file_sniffer() : wrapper for sniff_loop
//  - print() : writes stored packets to command line or file
class Capture{
    public:
        enum CaptureType{IS_SIP=0, IS_RTP, IS_OTHER};
        bool loop_stop = false; 
        
        Capture(CaptureType c,const std::string& filename);
        void run_sniffer(Tins::Sniffer& sniffer);
        void run_file_sniffer(Tins::FileSniffer& fsniffer);
        void print() const;
        void print(std::string& path) const;

        std::vector<Rtp> get_rtp_packets();
        std::vector<Sip> get_sip_packets();
        std::map<std::pair<std::string, std::string>,
            std::pair<std::string,std::string>> get_ports();
    private:
        std::unique_ptr<Tins::PacketWriter> p_writer;

        //sip packets
        std::vector<Sip> packets_;    
        //rtp packets
        std::vector<Rtp> rtp_packets_;

        //map for RTP ip and port 
        std::map<
            std::pair<std::string,std::string>,
            std::pair<std::string,std::string>
                > rtp_ips_and_ports;

        bool  capture_sip, capture_rtp;

        bool callback(const Tins::PDU& pdu);
};

#endif
