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
    //如果产生终止信号（ctrl+ca），就把当前文件内容写入文件中
    signal(SIGINT, signalHandler);
    printLogo();
    printStartMessage();
}

/// @brief 处理事务内容
/// @param commandsQueue 事务中存在的redis语句
/// @return 返回执行事务内的多条语句组成的结果
std::string RedisServer::executeTransaction(std::queue<std::string> &commandsQueue){
    std::vector<std::string> responseMessagesList;
    while(!commandsQueue.empty()){
        std::string commandLine = std::move(commandsQueue.front());
        commandsQueue.pop();
        std::istringstream iss(commandLine);
        std::string command;
        std::vector<std::string> tokens;
        std::string responseMessage;

        while(iss >> command){
            tokens.push_back(command);
        }
        if(!tokens.empty()){
            command = tokens.front();
            std::shared_ptr<CommandParser> commandParser = flyweightFactory->getParser(command); //获取解析器
            try {
                responseMessage = commandParser->parse(tokens);
            } 
            catch (const std::exception& e) {
                //语句执行错误，意思是说，hset a 2,本来该条语句应该是这个格式HSET key field value，所以该条语句内容错误
                responseMessage = "Error processing command '" + command + "': " + e.what();
            }   
            responseMessagesList.emplace_back(responseMessage);
        }
    }
    std::string res = "";
    for(int i=0; i<responseMessagesList.size(); i++){
        std::string responseMessage = std::to_string(i+1) + ")" + responseMessage[i];
        res += responseMessage;
        if(i!=responseMessagesList.size()-1){
            res+="\n";
        } 
    }
    return res;
}

/// @brief 处理客户端发过来的信息最原始信息，即字符串形式
/// @param receiveData 客户端发送过来的字符串
/// @return 返回客户端的redis语句处理结果
std::string RedisServer::handleClient(std::string receiveData){
    size_t bytesRead = receiveData.size();
    if(bytesRead > 0){
        std::istringstream iss(receiveData);
        std::string command;
        std::vector<std::string> tokens; //redis指令行，按照空格分割
        while(iss>>command){
            tokens.push_back(command);
        }
        while(!tokens.empty()){
            command = tokens.front();
            std::string responseMessage;
            if(command == "quit" || command == "exit"){
                responseMessage = "stop";
                return responseMessage;
            }
            else if(command == "multi"){
                if(startMulti){
                    responseMessage = "Open the transaction repeatedly";
                    return responseMessage;
                }
                startMulti =true;
                //queue不支持clear,所以用这个办法，在上一个事务完成（exec或者discard语句后）
                //本次事务开始时需要清空命令队列
                std::queue<std::string> empty;
                std::swap(empty, commandsQueue);
                responseMessage = "OK";
                return responseMessage;
            }
            else if(command == "exec"){
                if(startMulti == false){
                    responseMessage = "No transaction is opened!";
                    return responseMessage;
                }
                startMulti = false;
                if(!fallback){
                    responseMessage = executeTransaction(commandsQueue);
                    return responseMessage;
                }
                else{
                    fallback =false;
                    responseMessage = "(error) EXECABORT Transaction discarded because of previous errors.";
                    return responseMessage;
                }
            }
            else if (command == "discard"){
                startMulti = false;
                fallback = false;
                responseMessage = "OK";
                return responseMessage;
            }
            else{
                if(!startMulti){
                    std::shared_ptr<CommandParser> commandParser = flyweightFactory->getParser(command);
                    if (commandParser==nullptr){
                        responseMessage = "Error: Command '" + command + "' not recognized.";
                    }
                    else{
                        try{
                            responseMessage = commandParser->parse(tokens);
                        }
                        catch(const std::exception &e){
                            responseMessage = "Error processing command '" + command + "': " + e.what();
                        }
                    }
                    return responseMessage;
                }
                else{
                    std::shared_ptr<CommandParser> commandParser = flyweightFactory->getParser(command);
                    if (commandParser==nullptr){
                        fallback = true;
                        responseMessage = "Error: Command '" + command + "' not recognized.";
                        return responseMessage;
                    }
                    else{
                        commandsQueue.emplace(receiveData);
                        responseMessage = "QUEUE";
                        return responseMessage;
                    }
                }
            }
        }
    }
    else{
        return "nil";
    }
    return "error";
}

void RedisServer::signalHandler(int sig){
    if(sig == SIGINT){
        CommandParser::getRedisHelper()->flush();
        exit(0);
    }
}