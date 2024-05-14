#ifndef PARSER_FLYWEIGHT_FACTORY
#define PARSER_FLYWEIGHT_FACTORY

#include <unordered_map>
#include "CommandParser.h"

class ParserFlyweightFactory{
public:
    std::shared_ptr<CommandParser> getParser(std::string);

private:
    std::shared_ptr<CommandParser> createCommandParser(std::string &command);

private:
    std::unordered_map<std::string, std::shared_ptr<CommandParser>> parserMaps;
};



#endif