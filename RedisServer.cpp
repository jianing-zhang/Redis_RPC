#include"RedisServer.h"

/// @brief 懒汉单例模式
/// @return 返回唯一的RedisServer对象
RedisServer* RedisServer::getInstance(){
    static RedisServer redis;
    return &redis;
}

/// @brief 设置为private，防止外部创建，使其具有唯一性
/// @param port redis服务器的端口
/// @param logFilePath  logo文件路径，作为首次登陆弹出的界面信息 
RedisServer::RedisServer(int port, const std::string& logFilePath) 
: port(port), logFilePath(logFilePath), flyweightFactory(new ParserFlyweightFactory()){
    pid = getpid();
}

/// @brief 替代字符串中的指定字符
/// @param text 输入的字符串
/// @param toReplaceText 需要被替换的字符串
/// @param newText 替换成该字符串
void RedisServer::replaceText(std::string &text, const std::string &toReplaceText, const std::string &newText){
    size_t start_pos = text.find(toReplaceText);
    while(start_pos != std::string::npos){
        text.replace(start_pos, toReplaceText.size(), newText);
        start_pos = text.find(toReplaceText, start_pos + newText.size());
    }
}

/// @brief 打印logo文件内容，其中的一些关键字会被替代
void RedisServer::printLogo(){
    std::ifstream ifs(logFilePath);
    if(!ifs.is_open()){
        std::cout<<"logoFilePath不存在"<<std::endl;
    }
    std::string line = "";
    while(std::getline(ifs, line)){
        replaceText(line, "PORT", std::to_string(port));
        replaceText(line, "PTHREAD_ID", std::to_string(pid));
        std::cout << line << std::endl;
    }
}

/// @brief 获取当前时间
/// @return 返回当前时间的字符串形式
std::string RedisServer::getDate(){
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm;
    localtime_r(&now_c, &local_tm);
    std::ostringstream oss;
    oss<<std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/// @brief 打印开始的基本信息
void RedisServer::printStartMessage(){
    std::string startMessage = "[PID] DATE # Server started.";
    std::string initMessage = "[PID] DATE * The server is now ready to accept connections on port PORT";

    replaceText(startMessage, "PID", std::to_string(pid));
    replaceText(startMessage, "DATE", getDate());
    replaceText(initMessage, "PORT", std::to_string(port));
    replaceText(initMessage, "PID", std::to_string(pid));
    replaceText(initMessage, "DATE", getDate());
}

/// @brief 开始步骤，打印基本信息，设置对终止信号的处理，即写入文件
void RedisServer::start(){
    //如果产生终止信号，就把当前文件内容写入文件中
    signal(SIGINT, signalHandler);
    printLogo();
    printStartMessage();
}

