#include <iostream>
#include <mppEncoder.hpp>
#include <opencv2/opencv.hpp>
#include "mpp_packet_impl.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>

#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

using namespace std;

int main()
{
    cv::VideoCapture cap = cv::VideoCapture("./z_1.mp4");
    if (!cap.isOpened())
        return 0;
    cv::Mat img = cv::imread("/home/rpdzkj/wjm/pinlingv2.3.1/pinlingv2.3/mppEncoder/build/2c.jpg");
    char *args[] = {
        "/home/rpdzkj/wjm/pinlingv2.3.1/pinlingv2.3/mpp_test/build/mpi_enc_test", // argv[0] 通常是程序名
        "-i",
        "/home/rpdzkj/wjm/pinlingv2.3.1/pinlingv2.3/mppEncoder/cif2.yuv",
        "-w",
        "1280",
        "-h",
        "720",
        "-t",
        "7",
        "-o",
        "01.h264",
        "-f",
        "65543",
        "-n",
        "1000"};
    int argc = sizeof(args) / sizeof(char *);
    CMppEncoder mpp_encoder(argc, args);
    mpp_encoder.Init();

    // const char *output_url = "rtsp://127.0.0.1:8553/stream"; // 你的RTSP服务器地址
    // AVFormatContext *ofmt_ctx = nullptr;
    // AVOutputFormat *ofmt = nullptr;
    // AVStream *out_stream = nullptr;
    // AVPacket pkt;
    // int ret;
    // // // 注册所有编解码器和设备
    // // avdevice_register_all();

    // // 网络模块初始化
    // avformat_network_init();

    // // 分配输出媒体上下文
    // avformat_alloc_output_context2(&ofmt_ctx, nullptr, "rtsp", output_url);
    // if (!ofmt_ctx)
    // {
    //     fprintf(stderr, "Could not create output context\n");
    //     return -1;
    // }

    // // ofmt = ofmt_ctx->oformat;

    // // 添加视频流（根据您的AVPacket的实际情况，这里假设您的AVPacket已经是H264编码的视频流）
    // out_stream = avformat_new_stream(ofmt_ctx, nullptr);
    // if (!out_stream)
    // {
    //     fprintf(stderr, "Failed allocating output stream\n");
    //     return -1;
    // }

    // // 设置流参数，这个需要根据你的编码器的实际输出来设置
    // out_stream->codecpar->codec_id = AV_CODEC_ID_H264;
    // out_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    // out_stream->codecpar->width = 1280;
    // out_stream->codecpar->height = 720;
    // // out_stream->codecpar->format = AV_PIX_FMT_YUV420P;
    // out_stream->time_base = {1, 60}; // 示例时间基数

    // 注意：这里的设置应该与你的AVPacket所属的编码格式一致

    // // 打开RTSP输出上下文
    // ret = avio_open(&ofmt_ctx->pb, output_url, AVIO_FLAG_WRITE);
    // if (ret < 0)
    // {
    //     std::cout << ret << std::endl;
    //     fprintf(stderr, "Could not open output URL '%s'\n", output_url);
    //     return -1;
    // }

    // // 写入文件头
    // ret = avformat_write_header(ofmt_ctx, nullptr);
    // if (ret < 0)
    // {
    //     fprintf(stderr, "Error occurred when opening output URL\n");
    //     return -1;
    // }

    cv::Mat frame;
    MppPacketImpl mpp_packet;
    for (int i = 0; i < 1000; i++)
    {
        cap >> frame;
        mpp_encoder.TestMppRun(frame, mpp_packet);
        // pkt.size = mpp_packet.size;
        // memcpy(pkt.data, mpp_packet.data, mpp_packet.size);
        // pkt.pts = mpp_packet.pts;
        // pkt.dts = mpp_packet.dts;

        // pkt.stream_index = out_stream->index;
        // av_packet_rescale_ts(&pkt, av_codec_get_pkt_timebase(codec), out_stream->time_base);
        // pkt.pos = -1;

        // ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        // if (ret < 0)
        // {
        //     fprintf(stderr, "Error muxing packet\n");
        //     break;
        // }
        // av_packet_unref(&pkt);

        cv::waitKey(16);
    }
    // 写入文件尾（发送RTSP TEARDOWN命令）
    // av_write_trailer(ofmt_ctx);

    // 清理
    // avio_close(ofmt_ctx->pb);
    // avformat_free_context(ofmt_ctx);
    return 0;
}