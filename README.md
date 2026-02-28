人脸识别考勤分析系统

本项目包含两个独立的C++程序，用于实现基于摄像头的人脸识别考勤，并结合大语言模型对考勤记录进行分析，生成考勤报告。



功能简介

face\_recognition.cpp

调用百度智能云人脸识别API，通过摄像头实时检测人脸并识别身份。识别成功后，在画面中显示用户ID和识别时间，并将识别记录（用户ID+时间）输出到控制台。该程序可用于考勤打卡，记录谁在什么时间出现。



deepseek\_AA.cpp

读取由人脸识别程序生成的考勤记录文件（或其他文本文件），结合用户指定的问题，调用DeepSeek Chat API对记录进行分析，并将分析结果保存为“考勤记录分析.txt”。例如，可以统计某人的出勤情况、异常考勤分析等。



依赖安装

通用依赖

C++11 编译器（g++ 7+ 或 clang）



CMake 3.10+



libcurl（用于HTTP请求）



jsoncpp（用于JSON解析）



人脸识别程序额外依赖

OpenCV（推荐 4.x 版本）



百度AI C++ SDK（需自行下载配置）



安装命令（Ubuntu/Debian示例）

sudo apt update

sudo apt install build-essential cmake libcurl4-openssl-dev libjsoncpp-dev libopencv-dev

百度AI SDK安装

访问百度AI开放平台，注册并创建人脸识别应用，获取AppID、API Key、Secret Key。



下载C++ SDK（通常是一个包含face.h和库文件的压缩包），解压后将头文件放入系统路径（如/usr/local/include），库文件放入/usr/local/lib，并更新动态链接库缓存。



在编译时链接必要的库（如-lcurl -ljsoncpp -lopencv\_\* -lbaidu\_face\_sdk，具体库名依SDK版本而定）。



在代码中修改对应的字符串（不推荐，尤其不要提交到Git仓库）。



编译



编译成功后，会生成两个可执行文件：face\_recognition 和 deepseek\_AA。



运行

1\. 人脸识别程序

./face\_recognition

程序会自动打开摄像头，实时检测人脸。识别成功后，控制台会输出类似：



例如：

张三,2025-03-21 14:35:22

按Ctrl+C终止程序。如需保存识别记录，可重定向输出到文件：



./face\_recognition > attendance.log



2\. 考勤分析程序

该程序需要两个命令行参数：要分析的文本文件路径 和 问题。



例如，已有考勤记录文件attendance.log，想统计张三的出勤天数：



./deepseek\_AA attendance.log "请统计张三的出勤天数，并列出具体日期"

程序会调用DeepSeek API，分析结果保存为考勤记录分析.txt，并在终端输出返回的JSON信息。



注意事项

摄像头权限：确保运行用户有权限访问摄像头（Linux下通常需加入video组）。



网络连接：两个程序都需要联网调用云端API，请确保网络畅通。



API调用限制：百度人脸识别和DeepSeek API都有调用频率和配额限制，请合理使用。



安全警告：代码中若硬编码了API密钥，请务必在提交到GitHub前移除，并使用环境变量或配置文件。



百度SDK集成：由于百度官方C++ SDK可能未预装在系统中，请根据实际SDK路径调整CMakeLists.txt中的链接参数。

