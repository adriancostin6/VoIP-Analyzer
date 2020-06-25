# Program is to be used for capturing VoIP conversations    

## The main feature is the parsing and creation of SIP signaling packets, with a side emphasis on capturing live RTP data and converting it into WAV output files. (credits: https://github.com/yutongqing/rtp_decoder)

## How it works:
    1. Capture mode 
    2. Packet crafting mode

## Capture mode

The program supports live packet capture or the parsing of already captured packets from a PCAP file.

1. Live capture mode
    1. Captures all incoming traffic and saves it to a PCAP file
    1. Parses the PCAP file two times to extract the SIP and RTP data
    1. Saves all of the SIP packets to separate output files located in the output folder
    1. Decodes and writes the RTP data to two separate WAV files, one for each speaker in the call
1. Packet crafting mode
    1. Reads user input either from a text file or from the command line
    1. Creates a SIP packet from the information provided
    1. Validates the packet by checking for the correct syntax and presence of mandatory headers (for SIP requests)
    1. If the packet is valid it sends it over the network to the specified address and port.

## How to build and run the application:

    1. From the root directory of the project, create a new folder using `mkdir build`
    1. Change directory into the build directory with `cd build`
    1. Run CMake using the `cmake ..` command
    1. Run Make using the `make` command 
    1. Building the project will generate a `cap` output file, which you can run using `sudo ./cap` (packet capture requires root privileges)
