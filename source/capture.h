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

        std::map<std::pair<std::string, std::string>,
            std::pair<std::string,std::string>> get_ports();
    private:
        std::unique_ptr<Tins::PacketWriter> p_writer;
        std::vector<Sip> packets_;    
        std::vector<Rtp> rtp_packets_;

        //map for RTP ip and port 
        std::map<
            std::pair<std::string,std::string>,
            std::pair<std::string,std::string>
                > rtp_ips_and_ports;

        //flags for constructor and printing
        //unsigned pkg_num;
        //bool isFileSniffer,
        bool  capture_sip, capture_rtp;

        bool callback(const Tins::PDU& pdu);
};

#endif
