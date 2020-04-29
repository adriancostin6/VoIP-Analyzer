#ifndef CAPTURE_H
#define CAPTURE_H
#include <vector>
#include <iostream>

#include <tins/tins.h>

#include "sip.h"

class Capture{
    public:
        Capture();
        void run(Tins::Sniffer& sniffer);
        void run(Tins::FileSniffer& fsniffer);
        void print() const;
        void print(std::string& path) const;

    private:
        std::vector<Sip> packets_;    
        bool callback(const Tins::PDU& pdu);
};

#endif
