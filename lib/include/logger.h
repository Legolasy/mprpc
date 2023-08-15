#pragma once
#include "lockqueue.h"
#include<string>

// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

// 框架提供的日志系统
enum LoggerLevel
{
    INFO,
    ERROR,
};
// 单例模式 日志
class Logger
{
public:
    // 获取日志单例
    static Logger& GetInstance();
    void SetLogLevel(LoggerLevel level);
    void Log(std::string msg);
private:
    int m_log_level; // 日志级别
    LockQueue<std::string>m_lockQueue; // 日志 缓冲队列

    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

};