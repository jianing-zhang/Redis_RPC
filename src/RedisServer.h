#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <signal.h>
#include "ParserFlyweightFactory.h"

const std::string MY_PROJECT_DIR_LOGO = "./logo";

/// @brief 懒汉单例模式
class RedisServer{
public:
    static RedisServer* getInstance();
    std::string handleClient(std::string receiveData); 
    void start();

private:
    RedisServer(int port=5555, const std::string& logFilePath = MY_PROJECT_DIR_LOGO);
    static void signalHandler(int sig);
    void printLogo();
    void printStartMessage();
    void replaceText(std::string &text, const std::string &toReplaceText, const std::string &replaceText);
    std::string getDate();
    std::string executeTransaction(std::queue<std::string> &commandsQueue);

private:
    std::unique_ptr<ParserFlyweightFactory> flyweightFactory; //享元工厂
    int port;
    std::atomic<bool> stop{false};
    pid_t pid;
    std::string logFilePath;
    bool startMulti = false;
    bool fallback = false;
    std::queue<std::string> commandsQueue;
};

#endif