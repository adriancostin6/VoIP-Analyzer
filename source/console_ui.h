#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#include <string>

struct ConsoleUi
{
    static const std::string intro_text;
    static const std::string help;
    static const std::string header_check ;
    static const std::string packet_type;
    static const std::string request_type;
    static const std::string request_uri;
    static const std::string sip_version;
    static const std::string via_field;
    static const std::string via_addr;
    static const std::string via_params;
    static const std::string max_forwards;
    static const std::string to_field_value;
    static const std::string from_field_value;
    static const std::string contact_field_value;
    static const std::string call_id_value;
    static const std::string cseq_val;
    static const std::string content_length;
    static const std::string content_type;
    static const std::string sdp_v;
    static const std::string sdp_o;
    static const std::string sdp_s;
    static const std::string sdp_t;
    static const std::string sdp_m;
    static const std::string sdp_a;
    static const std::string sdp_a_check;
    static const std::string has_sdp_body;
    static const std::string other_packet_type;
    static const std::string other_header_info;
};

#endif
