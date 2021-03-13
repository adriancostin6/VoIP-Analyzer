#include "decode.h"

#include "codec.h"


#define CODEC_PCMA 8
#define CODEC_PCMU 0

//add_to_buffer function
//
//adds Rtp packet to the buffer 
//parameters:
//  rtp_packet : Rtp& (Rtp packet to be added)
//  buff : list<Rtp>& (list where packet will be pushed)
void add_to_buffer(Rtp& rtp_packet, std::list<Rtp>& buff)
{
    //if the buffer is empty push a packet in
    if(buff.empty())
    {
        buff.push_back(rtp_packet);
        return;
    }

    //iterate over all buffer elements and find a place to insert
    //the Rtp packet based on sequence number
    auto it = buff.begin();
    for(; it != buff.end();)
    {
        //if sequence is greater, continue iteration
        if(rtp_packet.get_seq_no() > (*it).get_seq_no())
            ++it;
        //if sequence is smaller, insert packet
        //list inserts before the current iterator
        else if(rtp_packet.get_seq_no() < (*it).get_seq_no())
        {
            buff.insert(it,rtp_packet);
            return;
        }
        else return;
    }

    //if none of the above, insert packet to list
    buff.push_back(rtp_packet);
}


// get_buffer_packet function
//
// usage: get Rtp packet from buffer
Rtp get_buffer_packet(std::list<Rtp>& buff)
{
    auto it = buff.begin();

    //get pointer to begining of buffer and return it 
    auto res = *it;

    //remove element from the buffer
    buff.pop_front();
    return res;
}

void decode(
        const std::string& in_filename,
        const std::string& out_filename,
        std::string& src_ip,
        std::string& dst_ip,
        std::string& src_port,
        std::string& dst_port
        )
{

    //sniffer configuration
    Tins::SnifferConfiguration conf;
    conf.set_promisc_mode(true);
    conf.set_immediate_mode(true);

    //init filter to ips and ports
    std::string filter = "src host ";
    filter+= src_ip;
    filter += " && ";
    filter += "dst host ";
    filter+= dst_ip; 
    filter += " && ";
    filter += "src port ";
    filter += src_port;
    filter += " && ";
    filter += "dst port ";
    filter += dst_port; 

    //configure sniffer for RTP capture
    conf.set_filter(filter);
    Tins::FileSniffer fsniffer_rtp(in_filename, conf);
    Capture cap_rtp(Capture::CaptureType::IS_RTP, out_filename);
    cap_rtp.run_file_sniffer(fsniffer_rtp);

    //get all captured Rtp packets
    std::vector<Rtp> rtp_elems = cap_rtp.get_rtp_packets();

    //init out file name
#ifdef _WIN32
    std::string out_file = "../../audio/" +  out_filename + ".wav";
#else
    std::string out_file = "../audio/" + out_filename + ".wav";
#endif

    //open out file 
    FILE* fp = fopen(out_file.c_str(), "w");
    if(!fp)
    {
        printf("open file fail\n");
    }

    
    //set space in file for wav header
    wav_header wav_h;
    memset((void*)&wav_h, 0xff, sizeof(wav_h));
    fwrite((void*)&wav_h, 1, sizeof(wav_h), fp);


    //init pcm_len and sample rate
    uint32_t pcm_len = 0;
    uint32_t sample_rate = 0;

    //define buffer
    std::list<Rtp> buffer;


    //create codec object
    G711Codec c;
    
    //create Rtp object for the current packet
    Rtp current_packet;

    //get iterator pointing to start of vector
    auto itr = rtp_elems.begin();

    //add all elements to the buffer, in order and remove duplicates
    //loop until all rtp elements have been through the buffer
    while(true)
    {
        while(buffer.empty())
        {
            //add element to buffer
            for(; itr != rtp_elems.end();)
            {
                add_to_buffer(*itr, buffer);
                ++itr;
                break;
            }

            //stop condition for when buffer is empty but all rtp elements
            //have been passed through it 
            if(itr == rtp_elems.end())
                break;
        }

        //stop condition for packet processing loop
        if(itr == rtp_elems.end())
            break;

        //get packet from buffer
        current_packet = get_buffer_packet(buffer);

        printf("RTP packet: [payload: %u, seq: %u, timestamp: %u, data len: %lu]\n",
                current_packet.get_payload_type(),
                current_packet.get_seq_no(),
                current_packet.get_timestamp(),
                current_packet.get_data().size()
                );

        //if payload type is not found skip the packet
        if(current_packet.get_payload_type() != CODEC_PCMA)
            if(current_packet.get_payload_type() != CODEC_PCMU)
            {
                std::cout<< "Codec not supported.\nPayload type is: " << current_packet.get_payload_type()<<"\n";            
                continue;
            }

        //get sample rate from codec
        sample_rate = c.get_sample_rate();

        //get decoded result
        std::string &&result = c.decode(
                current_packet.get_payload_type(),
                current_packet.get_data()
                );

        //write result to file and add it to pcm length
        fwrite(result.c_str(), 1, result.size(), fp);
        pcm_len += result.size();

    };

    //set "RIFF" chunk descriptor
    memcpy(wav_h.chunk_id, "RIFF", 4);
    memcpy(wav_h.format, "WAVE", 4);
    wav_h.chunk_size = sizeof(wav_h) + pcm_len - 8;

    //set fmt subchunk
    memcpy(wav_h.fmt_chunk.sub_chunk_id, "fmt ", 4);
    wav_h.fmt_chunk.sub_chunk_size = sizeof(wav_fmt_chunk) - 8;
    wav_h.fmt_chunk.audio_format = 1;
    wav_h.fmt_chunk.num_channels = 1;
    wav_h.fmt_chunk.sample_rate = sample_rate;
    wav_h.fmt_chunk.byte_rate = wav_h.fmt_chunk.sample_rate * 2;
    wav_h.fmt_chunk.block_align = 2;
    wav_h.fmt_chunk.bits_per_sample = 16;

    //set data subchunk
    memcpy(wav_h.data_chunk.sub_chunk_id, "data", 4);
    wav_h.data_chunk.sub_chunk_size = pcm_len;

    //go to start of file
    fseek(fp, 0, SEEK_SET);
    fwrite((void*)&wav_h, 1, sizeof(wav_h), fp);//write wav header
    printf("Finished writing WAV file with header size: %d\n", sizeof(wav_h));

    fclose(fp);

}
