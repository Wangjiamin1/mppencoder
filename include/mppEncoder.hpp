#ifndef _MPPENCODER_HPP_
#define _MPPENCODER_HPP_

#include "common.h"
#include <opencv2/opencv.hpp>
#include "mpp_packet_impl.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>

#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

class CMppEncoder
{
public:
    CMppEncoder(int argc, char **argv);
    ~CMppEncoder();
    void Init();
    int init_rtmp_streamer(char *stream, uint8_t *data, uint32_t size);
    MPP_RET TestMppRun(cv::Mat img);
    bool writeHeader(MpiEncTestData *mpp_enc_data, SpsHeader *sps_header);
    int write_frame(uint8_t *data, int size, MppPacketImpl *);

public:
    MpiEncMultiCtxInfo *info;

private:
    MPP_RET TestCtxInit();
    void TestCtxDeinit();
    MPP_RET TestMppEncCfgSetup();

    float total_rate;
    DataCrc checkcrc;
    AVPacket *pkt;
    AVStream *out_stream;
    AVFormatContext *ofmt_ctx = NULL;
    uint64_t frame_index = 0;
    SpsHeader sps_header;
};

#endif //  _MPPENCODER_HPP_
