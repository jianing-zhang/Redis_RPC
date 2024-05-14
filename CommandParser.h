#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <string>
#include <vector>
#include <memory>
#include "RedisHelper.h"


/// @brief 接口类，实现命令解析，具体实现需要继承这个
class CommandParser{
public:
    static void setRedisHelper(std::shared_ptr<RedisHelper> helper) { redisHelper = helper; }
    static std::shared_ptr<RedisHelper> getRedisHelper(){ return redisHelper; } 
    virtual std::string parse(std::vector<std::string> &tokens) = 0;
protected:
    static std::shared_ptr<RedisHelper> redisHelper;//静态成员变量，所有解析器共享一个RedisHelper 
};

/// @brief select行为解析类
class SelectParser : public CommandParser{
public:
    std::string parse(std::vector<std::string> &tokens) override;
};

/// @brief set行为解析累
class SetParser : public CommandParser{
public:
    std::string parse(std::vector<std::string> &tokens) override;
};

// SetnxParser 
class SetnxParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// SetexParser 
class SetexParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// GetParser 
class GetParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// KeysParser 
class KeysParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// DBSizeParser 
class DBSizeParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// ExistsParser 
class ExistsParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// DelParser 
class DelParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// RenameParser 
class RenameParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// IncrParser 
class IncrParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// IncrbyParser 
class IncrbyParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// IncrbyfloatParser 
class IncrbyfloatParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// DecrParser 
class DecrParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// DecrbyParser 
class DecrbyParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// MSetParser 
class MSetParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// MGetParser 
class MGetParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// StrlenParser 
class StrlenParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// AppendParser 
class AppendParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// LPushParser
class LPushParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// RPushParser
class RPushParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// LPopParser
class LPopParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// RPopParser
class RPopParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

//LRangeParser
class LRangeParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// HSetParser
class HSetParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// HGetParser
class HGetParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// HDelParser
class HDelParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// HKeysParser
class HKeysParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};

// HValsParser
class HValsParser : public CommandParser {
public:
    std::string parse(std::vector<std::string>& tokens) override;
};



#endif