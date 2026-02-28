#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <face.h>          // 百度AI人脸识别库头文件

using namespace cv;
using namespace std;
using namespace aip;        // 百度AI命名空间

int main()
{
    // 定义图像变量
    Mat img;                // 原始摄像头帧
    Mat grayImag;           // 灰度图像
    Mat equalizeImag;       // 直方图均衡化后的图像

    // 加载OpenCV自带的人脸检测级联分类器模型（Haar特征）
    CascadeClassifier Classifier("/usr/share/opencv4/haarcascades/haarcascade_frontalface_alt2.xml");
    
    vector< Rect > allFace; // 存储检测到的人脸矩形框
    Mat faceImag;           // 截取的人脸区域图像
    vector<uchar> jpgBuf;   // 用于存储JPEG编码后的图像数据
    
    // 时间相关变量，用于记录识别时刻
    time_t sec;
    tm * local_time;
    char buffer[64] = {0};
    
    // 百度AI平台申请的APPID、API Key和Secret Key
    std::string app_id = "APPID";
    std::string api_key = "API Key";
    std::string secret_key = "Secret Key";

    // 创建百度AI人脸客户端对象
    aip::Face client(app_id, api_key, secret_key);
    
    std::string base64Imag;          // 存储人脸图像的base64编码字符串
    Json::Value result;              // 存储百度API返回的JSON结果
    
    // 打开默认摄像头（设备0）
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        // 摄像头打开失败则直接退出
        return -1;
    }
    // 无限循环读取视频流
    for (;;)
    {
        cap.read(img);                // 从摄像头读取一帧
        cvtColor (img, grayImag, COLOR_RGBA2GRAY);   // 转换为灰度图
        equalizeHist (grayImag, equalizeImag);       // 直方图均衡化，增强对比度
        Classifier.detectMultiScale(grayImag, allFace); // 检测人脸，结果存入allFace
        
        if(allFace.size())            // 如果检测到至少一个人脸
        {
            // 在均衡化后的图像上绘制第一个检测到的人脸矩形框（颜色为青黄）
            rectangle(equalizeImag, allFace[0], Scalar(255,255,0));
            // 截取该人脸区域（仍为灰度图）
            faceImag = equalizeImag(allFace[0]);
            // 将人脸图像编码为JPEG格式，数据存入jpgBuf
            imencode(".jpg", faceImag, jpgBuf);
            
            // 将JPEG缓冲区数据进行base64编码
            base64Imag = base64_encode((char *)jpgBuf.data(), jpgBuf.size());
            
            // 调用百度人脸搜索接口（v3版本），在指定用户组（"1,user"）中搜索相似人脸
            result = client.face_search_v3(base64Imag, "BASE64", "1,user", aip::json_null);
            
            // 检查返回结果中是否包含有效数据
            if( !result["result"].isNull() )
            {
                // 如果相似度得分大于80（注意：此处if后误加分号，导致后续代码无条件执行）
                if( result["result"]["user_list"][0]["score"].asInt() > 80)
                {
                    // 获取当前系统时间并格式化为字符串
                    sec = time(NULL);
                    local_time = localtime(&sec);
                    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
                    // 输出用户ID和识别时间到控制台
                    cout << result["result"]["user_list"][0]["user_id"] << "," << std::string(buffer) << endl;
                    // 在图像上绘制用户ID（位置(0,50)）和时间（位置(0,100)）
                    putText(equalizeImag, result["result"]["user_list"][0]["user_id"].asString(), Point(0,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255));
                    putText(equalizeImag, std::string(buffer), Point(0,100), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255));
                }
            }
        }
        /*else
        {
            cout << "未检测到人脸" << endl;
        }*/
        // 显示处理后的图像（带人脸框和识别结果）
        imshow("video", equalizeImag);
        // 等待40ms，同时允许用户按键（若按下任意键可退出循环，但此处未判断返回值）
        waitKey(40) ;
    }
    return 0;
}
