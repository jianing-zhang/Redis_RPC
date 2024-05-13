#ifndef PARSER_FLYWEIGHT_FACTORY
#define PARSER_FLYWEIGHT_FACTORY

#include "CommandParser.h"


class ParserFlyweightFactory{
public:
    std::shared_ptr<CommandParser> getParser(std::string);
};



#endif