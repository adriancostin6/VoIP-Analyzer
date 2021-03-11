#include "console_ui.h"

#include "timer.h"

///////////////////////////////////////////////////////////////////////////////
//TEXT USER INTERFACE STRINGS
///////////////////////////////////////////////////////////////////////////////
const std::string ConsoleUi::intro_text = R"(
################################################################################
################################################################################
#                       WELCOME TO THE SIP PACKET CREATOR                      #
################################################################################
#   To start packet creation type: start                                       #
#   For help type: help                                                        #
################################################################################
)";
const std::string ConsoleUi::help = R"(
################################################################################
# The SIP packet must be valid according to the structure defined in RFC3261.  #
# To view this standard please visit: https://tools.ietf.org/html/rfc3261      #
#                                                                              #
# Standard for SDP message body: https://tools.ietf.org/html/rfc4566#section-5 #
################################################################################
#   Abbreviations:                                                             #
#       1. SP - Space                                                          #
#       2. CRLF - Carriage return, line feed                                   #
################################################################################
#                               PACKET STRUCTURE                               #
################################################################################
#   The basic structure of a SIP packet is shown below:                        #
#                                                                              #
#                generic-message  =  start-line                                #
#                                    *message-header                           #
#                                    CRLF                                      #
#                                    [ message-body ]                          #
#                start-line       =  Request-Line / Status-Line                #
################################################################################
#   The format of the request/status line is shown below:                      #
#                                                                              #
#           Request-Line  =  Method SP Request-URI SP SIP-Version CRLF         #
#           Status-Line  =  SIP-Version SP Status-Code SP Reason-Phrase CRLF   #
################################################################################
#   The format of a message header is shown below:                             #
#                                                                              #
#                       header-field: header-value                             #
#                                                                              #
#   The header field name should always be followed by a colon(:) or an equals #
#   sign(=) in case of message body headers. Whitespace should be added after  #
#   the delimiter and the header-value.                                        #
#                                                                              #
#   Each header field value can be followed by a number of additional          #
#   parameters, separated by semicolons(;).                                    #
################################################################################
#                                   REQUESTS                                   #
################################################################################
#   The minimally required header fields for a valid request are show below:   #
#                                                                              #
#                   Method SP Request-URI SP SIP-Version CRLF                  #
#                                                                              #
#   Each request has a number of six mandatory header fields.                  #
#                                                                              #
#       Via: SIP/Ver/Transport-Protocol net-addr(or hostname)[:port];params    #
#           - each Via header must contain a branch parameter of the format:   #
#                              branch=z9hG4bK...;                              #
#       To: ["Display name"] <sip_uri;params>;params                           #
#           - the To header field may or may not contain a display name        #
#           - the To header field must have a sip_uri with optional uri params #
#           - the To header field may contain optional parameters              #
#                                                                              #
#       From: same structure as To header field                                #
#                                                                              #
#       Contact: same structure as To header field                             #
#           - mandatory only in INVITE request                                 #
#                                                                              #
#       Call-Id: unique string to identify the call                            #
#       Max-Forwards: recommended value is 70                                  #
#       CSeq: Sequence_number Method                                           #
#                                                                              #
# These headers can be arranged in any particular order, but it is recommended #
# to store them in an order that facilitates easier parsing, meaning that      #
# the ones that are needed for proxy processing should be added first.         #
# The headers used for proxy processing are: Via, Route, Record-Route,         #
# Proxy-Require, Max-Forwards, Proxy-Authorization ..etc.                      #
#                                                                              #
#       The 6 most basic method types for requests are:                        #
#           1. INVITE                                                          #
#           2. ACK                                                             #
#           3. BYE                                                             #
#           4. CANCEL                                                          #
#           5. REGISTER                                                        #
#           6. OPTIONS                                                         #
#                                                                              #
#   The structure for a valid SIP URI is shown below:                          #
#                                                                              #
#                       sip:user[:password]@host[:port];params                 #
#                                                                              #
#   The uri identifier can be either sip, sips or tel                          #
#   The user field can be a username, an extension, or a phone number          #
#   A password can be included in the URI but it's not recommended.            #
#   The host can be a network address or domain name.                          #
#   A port value may or may not be included.                                   #
#   Additional parameters may or may not be included.                          #
################################################################################
#                                   RESPONSES                                  #
################################################################################
# Sip status messages are constructed by the User Agent Server(UAS) in order   #
# to provide a response to the request sent by the User Agent Client.          #
#                                                                              #
# They must contain a status-line listing the response code and message.       #
# The structure of the header for the response packet contains most of the     #
# same header fields that were present in the request sent to the server prior #
# to receiving the response packet.                                            #
################################################################################
#                                   DISCLAIMER                                 #
################################################################################
# This packet creator does not support parsing of response messages because    #
# the general use case for creating SIP packets is to send a request to the    #
# server for debugging or testing purposes.                                    #
#                                                                              #
# Nonetheless, if the user so wishes he/she may also create SIP responses with #
# the understanding that these will not be parsed for validity by the program. #
#                                                                              #
# Also, if the user wants to, he/she can also create other request packages    #
# that are not present in the 6 most common ones.                              #
#                                                                              #
################################################################################
# In order for these packets to go through and be stored, the user MUST use    #
# the packet creator with the VALIDITY CHECK turned OFF.                       #
################################################################################
# PROCEED TO PACKET CREATION? (yes/no)                                         #
################################################################################
)";
const std::string ConsoleUi::header_check = R"(
################################################################################
# Turn header check on? (yes/no)                                             #
################################################################################
# Leave the checker off if you want to construct a non standard request packet #
# or a response packet, or if you want to introduce certain errors for testing #
# purposes.                                                                    #
################################################################################
)";
const std::string ConsoleUi::packet_type = R"(
################################################################################
# Enter packet type. (expected: request or response)                           #
################################################################################
)";
const std::string ConsoleUi::response_type = R"(
################################################################################
# Enter response line.                                                         #
# (expected: SIP-Version SP Status-Code SP Reason-Phrase CRLF                  #
################################################################################
)";
const std::string ConsoleUi::request_type = R"(
################################################################################
# Enter request type. (expected: INVITE, ACK, REGISTER, OPTIONS, BYE, CANCEL)  #
################################################################################
)";
const std::string ConsoleUi::request_uri = R"(
################################################################################
# Enter request uri. (expected: sip:user[:password]@host[:port];params)        #
################################################################################
)";
const std::string ConsoleUi::sip_version = R"(
################################################################################
# Enter sip version. expected (SIP/2.0)                                        #
################################################################################
)";
const std::string ConsoleUi::via_field = R"(
################################################################################
# Enter header fields below.                                                   #
################################################################################
# Via header field.                                                            #
################################################################################
# Enter sip version and protocol used. (expected SIP/2.0/TCP, UDP, TLS, SCTP) #
################################################################################
)";
const std::string ConsoleUi::via_addr = R"(
################################################################################
# Enter via address. (expected: net-addr(or hostname)[:port])           #
################################################################################
)";
const std::string ConsoleUi::via_params = R"(
################################################################################
# Enter via parameters. (expected: branch=z9hG4bK...;params)                   #
################################################################################
)";
const std::string ConsoleUi::max_forwards = R"(
################################################################################
# Enter max-forwards value. (expected: 70, lower values need validation=off)   #
################################################################################
)";
const std::string ConsoleUi::to_field_value = R"(
################################################################################
# Enter To header value. (expected: ["Display name"] <sip_uri;params>;params)  #
################################################################################
)";
const std::string ConsoleUi::from_field_value = R"(
################################################################################
# Enter From header value. (expected: ["Display name"] <sip_uri;params>;params)#
################################################################################
)";
const std::string ConsoleUi::contact_field_value = R"(
################################################################################
# Enter Contact value. (expected: ["Display name"] <sip_uri;params>;params)    #
################################################################################
)";
const std::string ConsoleUi::call_id_value = R"(
################################################################################
# Enter Call-ID value. (expected: unique random string)                        #
################################################################################
)";
const std::string ConsoleUi::cseq_val = R"(
################################################################################
# Enter CSeq value. (expected: sequence_number METHOD(should match request)    #
################################################################################
)";
const std::string ConsoleUi::content_type = R"(
################################################################################
# Enter Content-Type value. (expected: application/sdp for sdp data)           #
################################################################################
)";
const std::string ConsoleUi::content_length = R"(
################################################################################
# Enter Content-Length value. (expected: length of sdp message body)           #
################################################################################
)";
const std::string ConsoleUi::sdp_v = R"(
################################################################################
# Enter SDP version value. (expected: v=0)                                     #
################################################################################
)";
const std::string ConsoleUi::sdp_o = R"(
################################################################################
# Enter SDP origin  value.                                                     #
# expected: o=<username> <sess-id> <sess-version> <nettype>                    #
#             <addrtype> <unicast-address>                                     #
################################################################################
)";
const std::string ConsoleUi::sdp_s = R"(
################################################################################
# Enter SDP session value. (expected: s=session-name)                          #
################################################################################
)";
const std::string ConsoleUi::sdp_t = R"(
################################################################################
# Enter SDP timing information. (expected: t=<start time> <stop time>)         #
################################################################################
)";
const std::string ConsoleUi::sdp_m = R"(
################################################################################
# Enter SDP media information. (expected: m=<media> <port> <proto> <fmt> ...)  #
################################################################################
)";
const std::string ConsoleUi::sdp_a = R"(
################################################################################
# Enter additional SDP information below. (expected: <type>=<value>)           #
################################################################################
)";
const std::string ConsoleUi::sdp_a_check = R"(
################################################################################
# Do you want to insert additional SDP headers? (yes/no)                       #
################################################################################
)";
const std::string ConsoleUi::other_packet_type = R"(
################################################################################
# Insert packet request/response line and header fields line by line.          #
################################################################################
)";
const std::string ConsoleUi::has_sdp_body = R"(
################################################################################
# Do you want to add a SDP message body? (yes/no)                              #
################################################################################
)";
const std::string ConsoleUi::other_header_info = R"(
################################################################################
# Do you want to add other optional header fields? (yes/no)                    #
################################################################################
)";
///////////////////////////////////////////////////////////////////////////////
//END TEXT USER INTERFACE STRINGS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//MAIN USER INTERFACE
///////////////////////////////////////////////////////////////////////////////
const std::string ConsoleUi::main_intro_text = R"(
################################################################################
#                               VoIP Analyser                                  #
################################################################################
#  To start a live packet capture: start capture                               #
#  To read packets from a pcap file: read pcap file                            #
#  To start the packet creator: packet creator                                 #
################################################################################
#  Input selected mode below                                                   #
################################################################################
)";
const std::string ConsoleUi::read_pcap = R"(
################################################################################
#  Enter PCAP filename stored in ../inputs  below                              #
################################################################################
)";
const std::string ConsoleUi::read_pcap_type = R"(
################################################################################
#  Provide additional information about the contents of the PCAP file          #
################################################################################
#  Parse only SIP packets? Enter: sip                                          #
#  Parse only RTP packets? Enter: rtp                                          #
#  Parse both SIP and RTP packets? Enter: sip and rtp                          #
################################################################################
#  Disclaimer:                                                                 #
#  Input PCAP file must contain the specified packet type in order for the     #
#  processing to be successful.                                                #
################################################################################
)";
const std::string ConsoleUi::capture_results= R"(
################################################################################
#  Generate SIP and audio output: output                                       #
#  Generate SIP output files: output sip                                       #
#  Generate audio output from RTP stream: output audio                         #
################################################################################
)";
const std::string ConsoleUi::packet_creator = R"(
################################################################################
#  To create a packet from input text file: input text file                    #
#  To create a packet from keyboard input: input keyboard                      #
################################################################################
)";
const std::string ConsoleUi::input_textfile = R"(
################################################################################
#  Enter name of text file below                                               #
################################################################################
)";
///////////////////////////////////////////////////////////////////////////////
//END MAIN USER INTERFACE
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//MAIN PROGRAM FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
void ConsoleUi::start()
{
    std::string user_input;
    std::cout << main_intro_text;

    //main program loop
    while(true) 
    {
        std::getline(std::cin, user_input);
        if(user_input == "start capture")
        {
            //get network interface
            Tins::NetworkInterface iface = 
                Tins::NetworkInterface::default_interface();
            Tins::NetworkInterface::Info info = iface.addresses();

            std::cout<< "Network interface: "<< iface.name()
                <<" IP ADDR: "<< info.ip_addr<< "\n";

            //sniffer configuration
            Tins::SnifferConfiguration config;
            config.set_promisc_mode(true);
            config.set_immediate_mode(true);

            try{
                Tins::Sniffer sniffer(iface.name(),config);

                Capture capture(Capture::IS_OTHER,"all_traffic");

                std::cout<<"Starting live capture. Press any key to stop\n";

                //start thread for capturing all packets until a key is pressed
                std::thread capture_all_traffic(
                        &Capture::run_sniffer, &capture, std::ref(sniffer)
                        );

                //stops program execution on this thread until a key is pressed
                //used to stop the capture loop 
                std::cin.get();

                //stop the capture
                capture.loop_stop = true;

                //wait for thread to finish 
                capture_all_traffic.join();

                std::cout<<capture_results;            

                //output while loop
                while(true)
                {
                    std::getline(std::cin, user_input);

                    if(user_input == "output")
                    {
                        //configure file sniffer to capture SIP packets
                        config.set_filter("port 5060");
                        Tins::FileSniffer sip_fsniffer(
                                "../temp/all_traffic.pcap",
                                config);

                        //start a timer for the SIP parsing
                        Timer t;
                        double time_taken_ms = 0;

                        //parse the file for SIP packets and write them to
                        //output files
                        std::cout<<"Generating SIP output files...\n";
                        Capture capture_sip(Capture::IS_SIP, "sip_packets");
                        capture_sip.run_file_sniffer(sip_fsniffer);
                        std::string output_path = "../outputs/sip/packet_";
                        capture_sip.print(output_path);
                        std::cout<<"SIP output files have been written to: "<< output_path << "\n";

                        //stop the timer and add the time to the result
                        time_taken_ms += t.stop();

                        //configure sniffer for to capture RTP packets
                        config.set_filter("udp[1] & 1 != 1 && udp[3] & 1 != 1 && udp[8] & 0x80 == 0x80 && length < 250");
                        Tins::FileSniffer fsniffer_rtp(
                                "../temp/all_traffic.pcap",
                                config);

                        //start a second timer for the RTP parser
                        Timer t1;

                        //parse the file for RTP packets
                        std::cout<<"Generating audio output files from RTP stream...\n";
                        Capture capture_rtp(Capture::IS_RTP,"rtp_packets");
                        capture_rtp.run_file_sniffer(fsniffer_rtp);
                        auto ports_and_ips = capture_rtp.get_ports();

                        //add timer stop value to the result
                        time_taken_ms += t1.stop();

                        //get server ip address
                        std::string server_ip;
                        std::cout<<"Enter PBX server IP address: ";
                        std::getline(std::cin,server_ip);

                        //start a third timer for the RTP decoding
                        Timer t2;

                        //erase the rtp data that is sent from the server to the 
                        //clients
                        auto it = ports_and_ips.begin();
                        for(; it!=ports_and_ips.end();)
                        {
                            if(it->first.first == server_ip)
                                ports_and_ips.erase(it++);
                            else
                                ++it; 
                        }

                        //list the ip address and port for the speakers which
                        //will have their RTP streams decoded
                        std::cout << "Speakers are listed below.\n";
                        uint8_t i = 1;
                        for(auto& key_pair : ports_and_ips)
                        {
                            auto ip_pair = key_pair.first;
                            auto port_pair = key_pair.second;

                            std::cout << "Speaker" << i;
                            std::cout << "Source IP: "<< ip_pair.first << " Destination IP: "<< ip_pair.second<< "\n";
                            std::cout << "Source Port: "<< port_pair.first << " Destination Port: "<< port_pair.second<< "\n";
                            i++;
                        }

                        //iterator for packet number
                        i=1;

                        //for each speaker, decode the rtp stream and write it
                        //to an output file
                        for(auto& key_pair : ports_and_ips)
                        {
                            std::string out_filename = "Speaker_" + std::to_string(i);
                            auto ip_pair = key_pair.first;
                            auto port_pair = key_pair.second;

                            decode(
                                    "../temp/rtp_packets.pcap", out_filename,
                                    ip_pair.first, ip_pair.second,
                                    port_pair.first, port_pair.second
                                  );
                            i++;
                        }

                        std::cout <<"Output audio files have been written to: ../audio\n"; 
                        time_taken_ms += t2.stop();
                        std::cout << "Time taken to process the packets: " << time_taken_ms <<"ms (" << time_taken_ms*1000 << "us)";
                        break;
                    }
                    else 
                        if(user_input == "output sip")
                        {
                            //configure file sniffer to capture SIP packets
                            config.set_filter("port 5060");
                            Tins::FileSniffer sip_fsniffer(
                                    "../temp/all_traffic.pcap",
                                    config);

                            //start timer for the SIP parser
                            Timer t;
                            double time_taken_ms = 0;

                            //parse the file for SIP packets and write the 
                            //output to a file
                            std::cout<<"Generating SIP output files...\n";
                            Capture capture_sip(Capture::IS_SIP, "sip_packets");
                            capture_sip.run_file_sniffer(sip_fsniffer);
                            std::string output_path = "../outputs/sip/packet_";
                            capture_sip.print(output_path);

                            std::cout<<"SIP output files have been written to: "<< output_path << "\n";
                            time_taken_ms += t.stop();
                            std::cout << "Time taken to process the packets: " << time_taken_ms <<"ms (" << time_taken_ms*1000 << "us)";
                            break;
                        }
                        else
                            if(user_input == "output audio")
                            {
                                //configure file sniffer to capture RTP packets
                                config.set_filter("udp[1] & 1 != 1 && udp[3] & 1 != 1 && udp[8] & 0x80 == 0x80 && length < 250");
                                Tins::FileSniffer fsniffer_rtp(
                                        "../temp/all_traffic.pcap",
                                        config);

                                //start a timer for the RTP parser
                                Timer t;
                                double time_taken_ms = 0;

                                //parse the file for RTP packets
                                std::cout<<"Generating audio output files from RTP stream...\n";
                                Capture capture_rtp(Capture::IS_RTP,"rtp_packets");
                                capture_rtp.run_file_sniffer(fsniffer_rtp);
                                auto ports_and_ips = capture_rtp.get_ports();

                                //add time to result
                                time_taken_ms += t.stop();

                                //get server IP
                                std::string server_ip;
                                std::cout<<"Enter PBX server IP address: ";
                                std::getline(std::cin,server_ip);

                                //start timer for RTP decoder
                                Timer t1;

                                //erase the rtp data that is sent from the server to the 
                                //client
                                auto it = ports_and_ips.begin();
                                for(; it!=ports_and_ips.end();)
                                {
                                    if(it->first.first == server_ip)
                                        ports_and_ips.erase(it++);
                                    else
                                        ++it; 
                                }

                                //list speakers which will have their RTP
                                //streams decoded
                                std::cout << "Speakers are listed below.\n";
                                uint8_t i = 1;
                                for(auto& key_pair : ports_and_ips)
                                {
                                    auto ip_pair = key_pair.first;
                                    auto port_pair = key_pair.second;

                                    std::cout << "Speaker" << i;
                                    std::cout << "Source IP: "<< ip_pair.first << " Destination IP: "<< ip_pair.second<< "\n";
                                    std::cout << "Source Port: "<< port_pair.first << " Destination Port: "<< port_pair.second<< "\n";
                                    i++;
                                }

                                //counter for output file number
                                i=1;

                                //for each speaker decode the RTP stream
                                //and add it to an output file
                                for(auto& key_pair : ports_and_ips)
                                {
                                    std::string out_filename = "Speaker_" + std::to_string(i);
                                    auto ip_pair = key_pair.first;
                                    auto port_pair = key_pair.second;

                                    decode(
                                            "../temp/rtp_packets.pcap", out_filename,
                                            ip_pair.first, ip_pair.second,
                                            port_pair.first, port_pair.second
                                          );
                                    i++;
                                }

                                std::cout <<"Output audio files have been written to: ../audio\n"; 
                                time_taken_ms += t1.stop();
                                std::cout << "Time taken to process the packets: " << time_taken_ms <<"ms (" << time_taken_ms*1000 << "us)";
                                break;
                            }
                            else
                                std::cout << "Error - Select a valid output method.\n";
                }
            }catch(std::exception& ex){
                std::cerr << "Error: " << ex.what() << "\n";
                return;
            }
            //if there were no errors, exit function
            return;
        }

        //does the same thing as the normal capture, but from a file
        if(user_input == "read pcap file")
        {
            //sniffer configuration
            Tins::SnifferConfiguration config;
            config.set_promisc_mode(true);
            config.set_immediate_mode(true);

            std::string file_name;
            std::cout << read_pcap;

            //get input file name
            std::getline (std::cin, file_name);
            std::string path = "../inputs/" + file_name + ".pcap";

            std::cout << read_pcap_type;

            while(true)
            {
                std::getline(std::cin, user_input);
                if(user_input == "sip")
                {

                    try{
                        //configure file sniffer to capture SIP packets
                        config.set_filter("port 5060");
                        Tins::FileSniffer sip_fsniffer(
                                path,
                                config);

                        //start timer for SIP parser
                        Timer t;
                        double time_taken_ms = 0;

                        //parse file for SIP packets
                        std::cout<<"Generating SIP output files...\n";
                        Capture capture_sip(Capture::IS_SIP, "sip_packets");
                        capture_sip.run_file_sniffer(sip_fsniffer);
                        std::string output_path = "../outputs/sip/packet_";
                        capture_sip.print(output_path);

                        std::cout<<"SIP output files have been written to: "<< output_path << "\n";
                        time_taken_ms += t.stop();
                        std::cout << "Time taken to process the packets: " << time_taken_ms <<"ms (" << time_taken_ms*1000 << "us)";
                        break;
                    }catch(std::exception& ex){
                        std::cerr<<"Error: "<< ex.what() << "\n";
                        return;
                    }

                }
                else
                    if(user_input == "rtp")
                    {

                        try{

                            config.set_filter("udp[1] & 1 != 1 && udp[3] & 1 != 1 && udp[8] & 0x80 == 0x80 && length < 250");
                            Tins::FileSniffer fsniffer_rtp(
                                    path,
                                    config);

                            //start timer for RTP parser
                            Timer t;
                            double time_taken_ms = 0;

                            //parse file for RTP packets
                            std::cout<<"Generating audio output files from RTP stream...\n";
                            Capture capture_rtp(Capture::IS_RTP,"rtp_packets");
                            capture_rtp.run_file_sniffer(fsniffer_rtp);
                            auto ports_and_ips = capture_rtp.get_ports();
                            time_taken_ms += t.stop();

                            //get server IP
                            std::string server_ip;
                            std::cout<<"Enter PBX server IP address: ";
                            std::getline(std::cin,server_ip);

                            //start timer for the RTP decoder
                            Timer t1;

                            //erase the rtp data that is sent from the server to the 
                            //client
                            auto it = ports_and_ips.begin();
                            for(; it!=ports_and_ips.end();)
                            {
                                if(it->first.first == server_ip)
                                    ports_and_ips.erase(it++);
                                else
                                    ++it; 
                            }

                            //list speakers which will have their RTP streams
                            //decoded
                            std::cout << "Speakers are listed below.\n";
                            uint8_t i = 1;
                            for(auto& key_pair : ports_and_ips)
                            {
                                auto ip_pair = key_pair.first;
                                auto port_pair = key_pair.second;

                                std::cout << "Speaker" << i;
                                std::cout << "Source IP: "<< ip_pair.first << " Destination IP: "<< ip_pair.second<< "\n";
                                std::cout << "Source Port: "<< port_pair.first << " Destination Port: "<< port_pair.second<< "\n";
                                i++;
                            }

                            //counter for output packet number
                            i=1;

                            //for each speaker decode the RTP stream and write
                            //output to file
                            for(auto& key_pair : ports_and_ips)
                            {
                                std::string out_filename = "Speaker_" + std::to_string(i);
                                auto ip_pair = key_pair.first;
                                auto port_pair = key_pair.second;

                                decode(
                                        "../temp/rtp_packets.pcap", out_filename,
                                        ip_pair.first, ip_pair.second,
                                        port_pair.first, port_pair.second
                                      );
                                i++;
                            }

                            std::cout <<"Output audio files have been written to: ../audio\n"; 
                            time_taken_ms += t1.stop();
                            std::cout << "Time taken to process the packets: " << time_taken_ms <<"ms (" << time_taken_ms*1000 << "us)";
                            break;
                        }catch(std::exception& ex){
                            std::cerr << "Error: " << ex.what() << "\n";
                            return;
                        }

                    }
                    else
                        if(user_input == "sip and rtp")
                        {
                            try{
                                //configure file sniffer for capturing SIP
                                config.set_filter("port 5060");
                                Tins::FileSniffer sip_fsniffer(
                                        path,
                                        config);

                                //start timer for SIP parser
                                Timer t;
                                double time_taken_ms = 0;

                                //parse the file for SIP packets and write
                                //the output to files
                                std::cout<<"Generating SIP output files...\n";
                                Capture capture_sip(Capture::IS_SIP, "sip_packets");
                                capture_sip.run_file_sniffer(sip_fsniffer);
                                std::string output_path = "../outputs/sip/packet_";
                                capture_sip.print(output_path);
                                std::cout<<"SIP output files have been written to: "<< output_path << "\n";

                                //add time value to result 
                                time_taken_ms += t.stop();

                                //configure file sniffer for capturing RTP
                                config.set_filter("udp[1] & 1 != 1 && udp[3] & 1 != 1 && udp[8] & 0x80 == 0x80 && length < 250");
                                Tins::FileSniffer fsniffer_rtp(
                                        path,
                                        config);

                                //start timer for RTP parser
                                Timer t1;

                                //parse the file for RTP packets
                                std::cout<<"Generating audio output files from RTP stream...\n";
                                Capture capture_rtp(Capture::IS_RTP,"rtp_packets");
                                capture_rtp.run_file_sniffer(fsniffer_rtp);
                                auto ports_and_ips = capture_rtp.get_ports();

                                //add time to result
                                time_taken_ms += t1.stop();

                                //get server IP
                                std::string server_ip;
                                std::cout<<"Enter PBX server IP address: ";
                                std::getline(std::cin,server_ip);

                                //start timer for RTP decoder
                                Timer t2;

                                //erase the rtp data that is sent from the server to the 
                                //client
                                auto it = ports_and_ips.begin();
                                for(; it!=ports_and_ips.end();)
                                {
                                    if(it->first.first == server_ip)
                                        ports_and_ips.erase(it++);
                                    else
                                        ++it; 
                                }

                                //list speakers which will have their RTP
                                //streams decoded
                                std::cout << "Speakers are listed below.\n";
                                uint8_t i = 1;
                                for(auto& key_pair : ports_and_ips)
                                {
                                    auto ip_pair = key_pair.first;
                                    auto port_pair = key_pair.second;

                                    std::cout << "Speaker" << i;
                                    std::cout << "Source IP: "<< ip_pair.first << " Destination IP: "<< ip_pair.second<< "\n";
                                    std::cout << "Source Port: "<< port_pair.first << " Destination Port: "<< port_pair.second<< "\n";
                                    i++;
                                }

                                //counter for output files
                                i=1;

                                //for each speaker decode the RTP stream and
                                //write the output to a file
                                for(auto& key_pair : ports_and_ips)
                                {
                                    std::string out_filename = "Speaker_" + std::to_string(i);
                                    auto ip_pair = key_pair.first;
                                    auto port_pair = key_pair.second;

                                    decode(
                                            "../temp/rtp_packets.pcap", out_filename,
                                            ip_pair.first, ip_pair.second,
                                            port_pair.first, port_pair.second
                                          );
                                    i++;
                                }

                                std::cout <<"Output audio files have been written to: ../audio\n"; 
                                time_taken_ms += t2.stop();
                                std::cout << "Time taken to process the packets: " << time_taken_ms <<"ms (" << time_taken_ms*1000 << "us)";
                                break;
                            }catch(std::exception& ex){
                                std::cerr << "Error: "<< ex.what() << "\n";
                                return;
                            }

                        }
                        else 
                            std::cout<<"Error - Select a valid packet type.\n";
            }
            //if there were no errors, exit function
            return;
        }
        if(user_input == "packet creator")
        {
            std::cout << packet_creator;
            while(true)
            {
                std::getline(std::cin,user_input);
                if(user_input == "input text file")
                {
                    //get input filename
                    std::string filename;
                    std::cout << "Enter packet data in a text file stored in ../inputs\n";
                    std::cout << "Enter name of text file stored in ../inputs: ";
                    std::getline(std::cin,filename);
                    std::string path = "../inputs/" + filename;

                    //construct PacketCrafter
                    PacketCrafter p;

                    //craft_sip_packet(filepath,packet num, check packet)
                    uint8_t p_num = 0;
                    p.craft_sip_packet(path, p_num, true);

                    while(true)
                    {
                        std::cout << "Craft another packet? (yes/no)";
                        std::getline(std::cin,user_input);

                        if(user_input == "yes")
                        {
                            std::cout << "Modify the input file with the new packet\n";
                            std::cout << "Enter: create\n";

                            std::getline(std::cin,user_input);
                            if(user_input != "create")
                                std::cout << "Error - Invalid input. Try again.\n";
                            else
                                p.craft_sip_packet(path, p_num, true);
                        }
                        else 
                            break;
                    }

                    //if there were no errors
                    p.send_packets();
                    break;
                }
                else
                    if(user_input == "input keyboard")
                    {
                        PacketCrafter p;

                        uint8_t p_num = 1;

                        p.get_user_input(p_num);
                        p_num++;
                        while(true)
                        {
                            std::cout<<"Create another packet? (yes/no)";
                            std::getline(std::cin,user_input);
                            if(user_input == "yes")
                            {
                                p.get_user_input(p_num); 
                                p_num++;
                            }
                            else
                                break;
                        }

                        //if there were no errors
                        p.send_packets(); 
                        break;
                    }
                    else
                        std::cout << "Error - Enter valid creation method.\n";
            }
            //if there were no errors, exit function
            return;
        }
        std::cout << "Error - Select a valid input method.\n"; 
    }
}
