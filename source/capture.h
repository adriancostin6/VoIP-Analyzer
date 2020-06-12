#ifndef CAPTURE_H
#define CAPTURE_H
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <map>

#include <tins/tins.h>

#include "sip.h"

class Capture{
    public:
        enum CaptureType{IS_SIP=0, IS_RTP, IS_OTHER};
        
        Capture(CaptureType c,const std::string& filename);
        void run(Tins::Sniffer& sniffer);
        void run(Tins::FileSniffer& fsniffer);
        void print() const;
        void print(std::string& path) const;

        std::map<std::pair<std::string, std::string>,
            std::pair<std::string,std::string>> getPorts();
    private:
        std::unique_ptr<Tins::PacketWriter> p_writer;
        std::vector<Sip> packets_;    

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
