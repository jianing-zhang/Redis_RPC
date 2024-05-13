#include <string>


class CommandParser{
public:
    virtual std::string parse(std::vector<std::string> &tokens) = 0;

};