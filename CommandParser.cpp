#include <stdexcept>
#include "CommandParser.h"


std::shared_ptr<RedisHelper> CommandParser::redisHelper = std::make_shared<RedisHelper>();

std::string SelectParser::parse(std::vector<std::string> &tokens){
    if(tokens.size() < 2){
        return "wrong number of arguments for SELECT.";
    }
    int index = 0;
    try {
        index = std::stoi(tokens[1]); //将字符串转换为整数
    } catch (std::invalid_argument const& e) { //如果转换失败
        return tokens[1] + " is not a numeric type"; //返回错误信息
    }
    return redisHelper->select(index); //调用RedisHelper的select方法
}