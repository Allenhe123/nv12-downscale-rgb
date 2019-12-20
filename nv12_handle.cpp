#include "nv12_handle.h"

#include <iostream>
#include <thread>

#include <QFileInfo>
#include <QFile>

#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"


// NV12 downscale
int NV12Scale(uint8 *psrc_buf, int psrc_w, int psrc_h, uint8 *pdst_buf, int pdst_w, int pdst_h, libyuv::FilterModeEnum pfmode)
{
    uint8 *i420_buf1 = (uint8 *)malloc(psrc_w * psrc_h * 3 / 2);
    uint8 *i420_buf2 = (uint8 *)malloc(pdst_w * pdst_h * 3 / 2);

    // NV12_1920x1080 -> I420_1920x1080
    libyuv::NV12ToI420(&psrc_buf[0],                        psrc_w,
                       &psrc_buf[psrc_w * psrc_h],          psrc_w,
                       &i420_buf1[0],                       psrc_w,
                       &i420_buf1[psrc_w * psrc_h],         psrc_w / 2,
                       &i420_buf1[psrc_w * psrc_h * 5 / 4], psrc_w / 2,
                       psrc_w, psrc_h);

    // I420_1920x1080 -> I420_1280x640
    libyuv::I420Scale(&i420_buf1[0],                       psrc_w,
                      &i420_buf1[psrc_w * psrc_h],         psrc_w / 2,
                      &i420_buf1[psrc_w * psrc_h * 5 / 4], psrc_w / 2,
                      psrc_w, psrc_h,
                      &i420_buf2[0],                       pdst_w,
                      &i420_buf2[pdst_w * pdst_h],         pdst_w / 2,
                      &i420_buf2[pdst_w * pdst_h * 5 / 4], pdst_w / 2,
                      pdst_w, pdst_h,
                      pfmode);

    // I420_1280x640 -> NV12_1280x640
    libyuv::I420ToNV12(&i420_buf2[0],                       pdst_w,
                       &i420_buf2[pdst_w * pdst_h],         pdst_w / 2,
                       &i420_buf2[pdst_w * pdst_h * 5 / 4], pdst_w / 2,
                       &pdst_buf[0],                        pdst_w,
                       &pdst_buf[pdst_w * pdst_h],          pdst_w,
                       pdst_w,pdst_h);

    free(i420_buf1);
    free(i420_buf2);

    return 0;
}



// NV12 downscale whole image
void downscale()
{
    using namespace std;
    const int src_w = 1920;
    const int src_h = 1080;
    const int dst_w = 1280;
    const int dst_h =  640;

    libyuv::FilterModeEnum fmode = libyuv::kFilterNone;

    char inputPathname[30]  = "/home/allen/nv12.dat";
    char outputPathname[30] = "/home/allen/nv12_1280X640.dat";

    FILE * fin  = fopen(inputPathname,  "r+");
    FILE * fout = fopen(outputPathname, "w+");

    // Allocate memory for nv12
    uint8* src_buf   = (uint8 *)malloc(src_w * src_h * 3 / 2);
    uint8* dst_buf   = (uint8 *)malloc(dst_w * dst_h * 3 / 2);
    uint8* rgb_buf   = (uint8 *)malloc(dst_w * dst_h * 3 * sizeof(uint8));

    // Read file data to buffer
    fread(src_buf, sizeof(uint8), src_w * src_h * 3 / 2, fin);

    // Scale NV12
    clock_t start = clock();
    for (int i=0; i < 10000; i++)
    {
        NV12Scale(src_buf, src_w, src_h, dst_buf, dst_w, dst_h, fmode);

        //color convert
        cv::Mat nv12(dst_h * 1.5, dst_w, CV_8UC1, dst_buf);
        cv::Mat rgb(dst_h,  dst_w, CV_8UC3, rgb_buf);
        cv::cvtColor(nv12, rgb, cv::COLOR_YUV2RGB_NV12);
    }

    clock_t finish = clock();
    float duration = (float)(finish-start)/CLOCKS_PER_SEC;
    printf("whole downscale time: %f\n", duration / 10000.0);

    //display
//    cout << rgb.rows << endl;
//    cout << rgb.cols << endl;

    // Write the data to file
    fwrite(dst_buf, sizeof(uint8), dst_w * dst_h * 3 /2, fout);

//    namedWindow("aa", cv::WINDOW_AUTOSIZE);
//    imshow("aa", rgb);
//    cv::waitKey(0);

    free(src_buf);
    free(dst_buf);
    free(rgb_buf);

    fclose(fin);
    fclose(fout);
}

// NV12 downscale as 3 parts 3:1:1
void downscale_part()
{
    using namespace std;
    const int src_w = 1920;
    const int src_h = 1080;
    const int dst_w = 1280;
    const int dst_h =  640;

    libyuv::FilterModeEnum fmode = libyuv::kFilterNone;

    char inputPathname[30]  = "/home/allen/nv12.dat";
    char outputPathname[30] = "/home/allen/nv12_1280X640.dat";

    FILE * fin  = fopen(inputPathname,  "r+");
    FILE * fout = fopen(outputPathname, "w+");

    // Allocate memory for nv12
    uint8 *src_buf   = (uint8 *)malloc(src_w * src_h * 3 / 2);
    uint8 *dst_buf   = (uint8 *)malloc(dst_w * dst_h * 3 / 2);
    uint8* rgb_buf   = (uint8 *)malloc(dst_w * dst_h * 3 * sizeof(uint8));

    // Read file data to buffer
    fread(src_buf, sizeof(uint8), src_w * src_h * 3 / 2, fin);

    uint8* src_buf1 = (uint8*)malloc(src_w * src_h * 0.6 * 3 / 2);
    uint8* src_buf2 = (uint8*)malloc(src_w * src_h * 0.2 * 3 / 2);
    uint8* src_buf3 = (uint8*)malloc(src_w * src_h * 0.2 * 3 / 2);

    uint8* dst_buf1   = (uint8*)malloc(dst_w * dst_h * 0.6 * 3 / 2);
    uint8* dst_buf2   = (uint8*)malloc(dst_w * dst_h * 0.2 * 3 / 2);
    uint8* dst_buf3   = (uint8*)malloc(dst_w * dst_h * 0.2 * 3 / 2);

    clock_t start1 = clock();

    for (int i=0; i < 10000; i++)
    {
        memcpy(src_buf1, src_buf, static_cast<uint32>(src_w * src_h * 0.6));
        memcpy(src_buf1 + static_cast<uint32>(src_w * src_h * 0.6), src_buf + src_w * src_h,  static_cast<uint32>(src_w * src_h * 0.6 * 0.5));

        memcpy(src_buf2, src_buf + static_cast<uint32>(src_w * src_h * 0.6),  static_cast<uint32>(src_w * src_h * 0.2));
        memcpy(src_buf2 + static_cast<uint32>(src_w * src_h * 0.2), src_buf + src_w * src_h + static_cast<uint32>(src_w * src_h * 0.6 * 0.5), static_cast<uint32>(src_w * src_h * 0.2 * 0.5));

        memcpy(src_buf3, src_buf + static_cast<uint32>(src_w * src_h * 0.8), static_cast<uint32>(src_w * src_h * 0.2));
        memcpy(src_buf3 + static_cast<uint32>(src_w * src_h * 0.2), src_buf + src_w * src_h + static_cast<uint32>(src_w * src_h * 0.8 * 0.5), static_cast<uint32>(src_w * src_h * 0.2 * 0.5));

        NV12Scale(src_buf1, src_w, src_h * 0.6, dst_buf1, dst_w, dst_h * 0.6, fmode);
        NV12Scale(src_buf2, src_w, src_h * 0.2, dst_buf2, dst_w, dst_h * 0.2, fmode);
        NV12Scale(src_buf3, src_w, src_h * 0.2, dst_buf3, dst_w, dst_h * 0.2, fmode);

        memcpy(dst_buf, dst_buf1, static_cast<uint32>(dst_w * dst_h * 0.6));
        memcpy(dst_buf + static_cast<uint32>(dst_w * dst_h * 0.6), dst_buf2, static_cast<uint32>(dst_w * dst_h * 0.2));
        memcpy(dst_buf + static_cast<uint32>(dst_w * dst_h * 0.8), dst_buf3, static_cast<uint32>(dst_w * dst_h * 0.2));
        memcpy(dst_buf + dst_w * dst_h, dst_buf1 + static_cast<uint32>(dst_w * dst_h * 0.6), static_cast<uint32>(dst_w * dst_h * 0.6 * 0.5));
        memcpy(dst_buf + dst_w * dst_h + static_cast<uint32>(dst_w * dst_h * 0.6 * 0.5), dst_buf2 + static_cast<uint32>(dst_w * dst_h * 0.2), static_cast<uint32>(dst_w * dst_h * 0.2 * 0.5));
        memcpy(dst_buf + dst_w * dst_h + static_cast<uint32>(dst_w * dst_h * 0.6 * 0.5 + dst_w * dst_h * 0.2 * 0.5), dst_buf3
               + static_cast<uint32>(dst_w * dst_h * 0.2), static_cast<uint32>(dst_w * dst_h * 0.2 * 0.5));

        //color convert
        cv::Mat nv12(dst_h * 1.5, dst_w, CV_8UC1, dst_buf);
        cv::Mat rgb(dst_h,  dst_w, CV_8UC3, rgb_buf);
        cv::cvtColor(nv12, rgb, cv::COLOR_YUV2RGB_NV12);
    }

    clock_t finish1 = clock();
    float duration1 = (float)(finish1 - start1)/CLOCKS_PER_SEC;
    printf("scale time1: %f\n", duration1 / 10000.0);

    //display
//    cout << rgb.rows << endl;
//    cout << rgb.cols << endl;

    // Write the data to file
    fwrite(dst_buf, sizeof(uint8), dst_w * dst_h * 3 /2, fout);

//    namedWindow("aa", cv::WINDOW_AUTOSIZE);
//    imshow("aa", rgb);
//    cv::waitKey(0);

    free(src_buf);
    free(dst_buf);
    free(src_buf1);
    free(src_buf2);
    free(src_buf3);
    free(dst_buf1);
    free(dst_buf2);
    free(dst_buf3);
    free(rgb_buf);

    fclose(fin);
    fclose(fout);
}

 // rate_h = 0.6
void thread_entry1(uint8* src_buf, uint32 src_w, uint32 src_h, float rate_h, uint32 dst_w, uint32 dst_h, uint8* rgb_out)
{
    uint8* src_buf1 = (uint8*)malloc(src_w * src_h * rate_h * 1.5);
    uint8* dst_buf1   = (uint8*)malloc(dst_w * dst_h * rate_h * 1.5);

    clock_t start1 = clock();
    for (int i=0; i < 10000; i++)
    {
        memcpy(src_buf1, src_buf, static_cast<uint32>(src_w * src_h * rate_h));
        memcpy(src_buf1 + static_cast<uint32>(src_w * src_h * rate_h), src_buf + src_w * src_h,
               static_cast<uint32>(src_w * src_h * rate_h * 0.5));

        NV12Scale(src_buf1, src_w, src_h * rate_h, dst_buf1, dst_w, dst_h * rate_h, libyuv::kFilterNone);
        //color convert
        cv::Mat nv12(dst_h * 1.5 * rate_h, dst_w, CV_8UC1, dst_buf1);
        cv::Mat rgb(dst_h * rate_h,  dst_w, CV_8UC3, rgb_out);
        cv::cvtColor(nv12, rgb, cv::COLOR_YUV2RGB_NV12);
    }
    clock_t finish1 = clock();
    float duration1 = (float)(finish1 - start1)/CLOCKS_PER_SEC;
    printf("thread scale time1: %f\n", duration1 / 10000.0);

    free(src_buf1);
    free(dst_buf1);
}


// rate_h1 = 0.6 rate_h2 = 0.2
void thread_entry2(uint8* src_buf, uint32 src_w, uint32 src_h, float rate_h1, float rate_h2, uint32 dst_w, uint32 dst_h, uint8* rgb_out)
{
     uint8* src_buf2 = (uint8*)malloc(src_w * src_h * rate_h2 * 1.5);
     uint8* dst_buf2   = (uint8*)malloc(dst_w * dst_h * rate_h2 * 1.5);

     clock_t start1 = clock();
     for (int i=0; i < 10000; i++)
     {
         memcpy(src_buf2, src_buf + static_cast<uint32>(src_w * src_h * rate_h1),  static_cast<uint32>(src_w * src_h * rate_h2));
         memcpy(src_buf2 + static_cast<uint32>(src_w * src_h * rate_h2),
                src_buf + src_w * src_h + static_cast<uint32>(src_w * src_h * rate_h1 * 0.5),
                static_cast<uint32>(src_w * src_h * rate_h2 * 0.5));

         NV12Scale(src_buf2, src_w, src_h * rate_h2, dst_buf2, dst_w, dst_h * rate_h2, libyuv::kFilterNone);

         //color convert
         cv::Mat nv12(dst_h * 1.5 * rate_h2, dst_w, CV_8UC1, dst_buf2);
         cv::Mat rgb(static_cast<uint32>(dst_h * rate_h2),  dst_w, CV_8UC3, rgb_out
                     + static_cast<uint32>(dst_h * rate_h1 * dst_w * sizeof(uint8) * 3));

         cv::cvtColor(nv12, rgb, cv::COLOR_YUV2RGB_NV12);
     }

     clock_t finish1 = clock();
     float duration1 = (float)(finish1 - start1)/CLOCKS_PER_SEC;
     printf("thread scale time2: %f\n", duration1 / 10000.0);

     free(src_buf2);
     free(dst_buf2);
}

// rate_h1 = 0.6 rate_h2 = 0.2 rate_h3 = 0.2
void thread_entry3(uint8* src_buf, uint32 src_w, uint32 src_h, float rate_h1, float rate_h2,
                   float rate_h3, uint32 dst_w, uint32 dst_h, uint8* rgb_out)
{
    uint8* src_buf3 = (uint8*)malloc(src_w * src_h * rate_h3 * 1.5);
    uint8* dst_buf3   = (uint8*)malloc(dst_w * dst_h * rate_h3 * 1.5);

    clock_t start1 = clock();

    for (int i=0; i < 10000; i++)
    {
        memcpy(src_buf3, src_buf + static_cast<uint32>(src_w * src_h * (rate_h1 + rate_h2)), static_cast<uint32>(src_w * src_h * rate_h3));
        memcpy(src_buf3 + static_cast<uint32>(src_w * src_h * rate_h3),
               src_buf + src_w * src_h + static_cast<uint32>(src_w * src_h * (rate_h1 + rate_h2) * 0.5),
               static_cast<uint32>(src_w * src_h * rate_h3 * 0.5));

        NV12Scale(src_buf3, src_w, src_h * rate_h3, dst_buf3, dst_w, dst_h * rate_h3, libyuv::kFilterNone);
        //color convert
        cv::Mat nv12(dst_h * 1.5 * rate_h3, dst_w, CV_8UC1, dst_buf3);
        cv::Mat rgb(static_cast<uint32>(dst_h * rate_h3),  dst_w, CV_8UC3,
                    rgb_out + static_cast<uint32>(dst_h * (rate_h1 + rate_h2) * dst_w * sizeof(uint8) * 3));

        cv::cvtColor(nv12, rgb, cv::COLOR_YUV2RGB_NV12);
    }

    clock_t finish1 = clock();
    float duration1 = (float)(finish1 - start1)/CLOCKS_PER_SEC;
    printf("thread scale time3: %f\n", duration1 / 10000.0);

    free(src_buf3);
    free(dst_buf3);
}

// NV12 downscale
void downscale_part_color_convert_thread()
{
    using namespace std;

    const uint32 src_w = 1920;
    const uint32 src_h = 1080;
    const uint32 dst_w = 1280;
    const uint32 dst_h =  640;

    char inputPathname[30]  = "/home/allen/nv12.dat";
    char outputPathname[30] = "/home/allen/nv12_1280X640.dat";

    FILE * fin  = fopen(inputPathname,  "r+");
    FILE * fout = fopen(outputPathname, "w+");

    // Allocate memory for nv12 and rgb
    uint8 *src_buf   = (uint8 *)malloc(src_w * src_h * 1.5);
    uint8 *rgb_buf   = (uint8 *)malloc(dst_w * dst_h * sizeof(uint8) * 3);

    // Read file data to buffer
    fread(src_buf, sizeof(uint8), src_w * src_h * 3 / 2, fin);

    std::thread t1(thread_entry1, src_buf, src_w, src_h, 0.6, dst_w, dst_h, rgb_buf);
    std::thread t2(thread_entry2, src_buf, src_w, src_h, 0.6, 0.2, dst_w, dst_h, rgb_buf);
    std::thread t3(thread_entry3, src_buf, src_w, src_h, 0.6, 0.2, 0.2, dst_w, dst_h, rgb_buf);
    if (t1.joinable())  t1.join();
    if (t2.joinable())  t2.join();
    if (t3.joinable())  t3.join();

    // Write the data to file
    fwrite(rgb_buf, sizeof(uint8), dst_w * dst_h * sizeof(uint8) * 3, fout);

    //display
    cv::Mat rgbMat(dst_h, dst_w, CV_8UC3, rgb_buf);
    cout << rgbMat.rows << endl;
    cout << rgbMat.cols << endl;

    namedWindow("aa", cv::WINDOW_AUTOSIZE);
    imshow("aa", rgbMat);
    cv::waitKey(0);


    free(src_buf);
    free(rgb_buf);
    fclose(fin);
    fclose(fout);
}



void nv12_to_rgb()
{
    using namespace std;
    using namespace cv;

    QString filename("/home/allen/nv12_1280X640.dat");
    uint32_t W = 1280;
    uint32_t H = 640;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QByteArray t = file.readAll();
    cout << t.size() << endl;
    file.close();

    cv::Mat nv12(H * 1.5, W, CV_8UC1, (uchar*)t.data());
    cv::Mat rgb(H, W, CV_8UC3);

    clock_t start = clock();
    cvtColor(nv12, rgb, COLOR_YUV2RGB_NV12);
    clock_t finish = clock();
    float duration = (float)(finish-start)/CLOCKS_PER_SEC;
    printf("scale time: %f\n", duration);   // 4ms

    cout << rgb.rows << endl;
    cout << rgb.cols << endl;

    namedWindow("aa",WINDOW_AUTOSIZE);
    imshow("aa", rgb);
    waitKey(0);
}
