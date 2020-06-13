#ifndef DECODE_H
#define DECODE_H

#include "pcap_reader.h"
#include "codec.h"



void decode(
        const std::string& in_filename,
        const std::string& out_filename,
        std::string& src_ip,
        std::string& dst_ip,
        std::string& src_port,
        std::string& dst_port
        );

#endif
