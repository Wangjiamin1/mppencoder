#ifndef _MPPENCODER_HPP_
#define _MPPENCODER_HPP_

#include "common.h"
#include <opencv2/opencv.hpp>
#include "mpp_packet_impl.h"

class CMppEncoder
{
public:
    CMppEncoder(int argc, char **argv);
    ~CMppEncoder();
    void Init();
    MPP_RET TestMppRun(cv::Mat img, MppPacketImpl &mpp_packet);

public:
    MpiEncMultiCtxInfo *info;

private:
    MPP_RET TestCtxInit();
    void TestCtxDeinit();
    MPP_RET TestMppEncCfgSetup();

    float total_rate;
    DataCrc checkcrc;
};

#endif //  _MPPENCODER_HPP_
