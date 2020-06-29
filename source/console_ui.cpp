#include "console_ui.h"

///////////////////////////////////////////////////////////////////////////////
//TEXT USER INTERFACE STRINGS
///////////////////////////////////////////////////////////////////////////////
    const  std::string ConsoleUi::intro_text = R"(
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


///////////////////////////////////////////////////////////////////////////////
//END MAIN USER INTERFACE
///////////////////////////////////////////////////////////////////////////////
