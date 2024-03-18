#include <iostream>
#include <mppEncoder.hpp>
#include <opencv2/opencv.hpp>
#include "mpp_packet_impl.h"

using namespace std;

int main()
{
    cv::VideoCapture cap = cv::VideoCapture("/home/rpdzkj/code/mppencoder/build/z_1.mp4");
    if (!cap.isOpened())
        return 0;
    // cv::Mat img = cv::imread("/home/rpdzkj/wjm/pinlingv2.3.1/pinlingv2.3/mppEncoder/build/2c.jpg");
    char *args[] = {
        "/home/rpdzkj/wjm/pinlingv2.3.1/pinlingv2.3/mpp_test/build/mpi_enc_test", // argv[0] 通常是程序名
        "-w",
        "1920",
        "-h",
        "1080",
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

    cv::Mat frame;
    while (cap.read(frame))
    {
        mpp_encoder.TestMppRun(frame);

        // cv::waitKey(16);
    }
    return 0;
}