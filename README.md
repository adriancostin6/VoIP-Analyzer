# VoIP Analyser

Command-line interface for capturing and analysing VoIP traffic. Main features
include the parsing and creation of SIP signaling packets, with a side emphasis
on capturing RTP data, decoding it and producing .wav files.

## Getting Started

In order to use this repository you will have to:
1. Clone to your location of choice 
    1. SSH: `git clone git@github.com:adriancostin6/VoIP-Analyser.git`
    1. HTTPS: `git clone https://github.com/adriancostin6/VoIP-Analyser.git`
1. For Linux
    1. Change directory to the project `cd VoIP-Analyser`
    1. Change directory to linux `cd linux`
    1. Make build directory `mkdir build`
    1. Change directory to build `cd build`
    1. Run CMake using `cmake ..`
    1. Run Make using `make`
    1. Run the generated executable using `sudo ./cap` because packet capture requires root privileges.
1. For Windows
    1. Open Visual Studio solution in win32 directory
    1. Build the project in Release/x64 or Debug/x64
    1. Run the generated executable

## Prerequisites

This project requires the [libtins](https://github.com/mfontanini/libtins) library for compilation. 
Instructions for installing this dependency are provided on the official github page.

The packet capture was run and tested on an Asterisk PBX server running inside a virtual machine.

## Features 

1. Live capture mode
    1. Live interface
        1. Captures all incoming traffic and saves it to a PCAP file
        1. Parses the PCAP file two times to extract the SIP and RTP data
        1. Saves all of the SIP packets to separate output files located in the output folder
        1. Decodes and writes the RTP data to two separate WAV files, one for each speaker in the call.
    1, Existing PCAP file
        1. Parses an existing PCAP file for SIP and RTP data as described above
1. Packet crafting mode
    1. Reads user input either from a text file or from the command line
    1. Creates a SIP packet from the information provided
    1. Validates the packet by checking for the correct syntax and presence of mandatory headers (for SIP requests)
    1. If the packet is valid it sends it over the network to the specified address and port

## Decoding RTP packets only works on the linux version at the moment.
