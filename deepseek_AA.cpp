#include <iostream>
#include <fstream>      // 文件输入输出流
#include <sstream>      // 字符串流
#include <iterator>     // 迭代器，用于读取文件
#include <string>       // 字符串类
#include <curl/curl.h>  // libcurl库，用于HTTP请求
#include <jsoncpp/json/json.h> // jsoncpp库，用于处理JSON数据

using namespace std;

// libcurl 写回调函数，将响应数据写入到 stringstream 中
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::stringstream *)userp)->write((char *)contents, size*nmemb);
    return size*nmemb;   // 返回实际处理的字节数，必须等于 size*nmemb 表示成功
}

int main(int argc, char* argv[])
{
    // 检查命令行参数数量，需要至少两个参数：文件路径和问题
    if(argc < 3)
        return 1;
        
    std::string filePath(argv[1]);   // 第一个参数：要读取的文件路径
    std::string question(argv[2]);    // 第二个参数：用户的问题

    //1 打开文件
    std::ifstream recordFile(filePath.c_str());
    if(!recordFile)
    {
        cout << "文件打开失败" << endl;
        return 1;
    }
    //2 读取文件内容到字符串 content 中（使用输入流迭代器）
    std::string content((std::istreambuf_iterator<char>(recordFile)), std::istreambuf_iterator<char>());
    //3 输出文件内容（调试用，此处被注释）
    //cout << content << endl;
    //4 关闭文件
    recordFile.close();
    //5 初始化 cURL
    CURL * curl = curl_easy_init();
    if(!curl)
    {
        cout << "Curl init 错误" << endl;
        return 1;
    }
    
    // API 请求的 URL 和请求头
    const std::string url = "https://api.deepseek.com/chat/completions";
    const std::string type = "Content-Type:application/json";
    const std::string Authorization = "deepseekAPI密钥"; // 此处为硬编码的 API 密钥
    
    struct curl_slist * headers = nullptr;
    headers = curl_slist_append(headers, type.c_str());        // 添加 Content-Type 头
    headers = curl_slist_append(headers, Authorization.c_str()); // 添加 Authorization 头

    // 构建请求体的 JSON 对象
    Json::Value request_body;
    request_body["model"] = "deepseek-chat";     // 使用的模型名称
    request_body["stream"] = false;               // 不使用流式响应

    // system 消息，设置助手角色
    Json::Value system_msg;
    system_msg["role"] = "system";
    system_msg["content"] = "You are a helpful assistant";

    // user 消息，将文件内容和用户问题拼接作为提示词
    Json::Value user_msg;
    std::string prompt = content+" "+question;
    user_msg["role"] = "user";
    user_msg["content"] = prompt;

    // 将消息列表放入数组
    Json::Value messages(Json::arrayValue);
    messages.append(system_msg);
    messages.append(user_msg);

    request_body["messages"] = messages;   // 将消息数组放入请求体

    // 可选：打印请求体（调试用）
    //cout << request_body << endl;
    
    // 将 JSON 对象转换为字符串（FastWriter 输出紧凑格式）
    Json::FastWriter writer;
    std::string post_data = writer.write(request_body);
    
    std::stringstream response_stream;   // 用于存储 API 响应的字符串流

    // 设置 libcurl 选项
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());          // 设置请求 URL
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);       // 设置请求头
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str()); // 设置 POST 数据
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // 设置写回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_stream);   // 将响应数据写入 response_stream
   
    // 执行 HTTP 请求
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        cout << "返回失败" << endl;
        curl_easy_cleanup(curl);   // 清理 curl 资源
        return 1;
    }
    
    // 输出原始响应流（调试用）
    cout << "response_stream" << response_stream.str() << endl;
    
    // 解析 JSON 响应
    Json::Value response_json;
    Json::CharReaderBuilder reader;
    std::string errs;
    
    // 从字符串流解析 JSON
    if(!Json::parseFromStream(reader, response_stream, &response_json, &errs))
    {
        std::cerr << "发生错误：" << errs << endl;
        curl_easy_cleanup(curl);
        return 1;
    }
    // 检查响应中是否包含错误字段
    if(response_json.isMember("error"))
    {
        std::cerr << "API 错误：" << response_json["error"]["message"].asString() << endl;
        curl_easy_cleanup(curl);
        return 1;
    }
    // 检查响应中是否有 choices 字段且不为空
    if(response_json.isMember("choices") && !response_json["choices"].empty())
    {
        // 提取第一个 choice 中的 message 对象
        Json::Value response_message = response_json["choices"][0]["message"];
        cout << "Role" << response_message["role"].asString() << endl;      // 输出角色
        cout << "Content" << response_message["content"].asString() << endl; // 输出内容

        // 将返回的内容保存到文件 "考勤记录分析.txt"
        std::ofstream reportFile("考勤记录分析.txt");
        if(reportFile.is_open())
        {
            reportFile << response_message["content"].asString();
            reportFile.close();
        }
        else
        {
            std::cerr << "分析失败" << endl;
        }
    }
    else
    {
        std::cerr << "不是期望格式" << endl;
    }
    
    // 释放请求头链表并清理 curl
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
   
    return 0;
}
