#include "logger.h"
#include <time.h>
#include<iostream>
Logger& Logger::GetInstance() {
  static Logger logger;
  return logger;
}
Logger::Logger()
{
    //启动写日志 线程
    std:: thread writeLog([&](){
        for(;;)
        {
            // 获取当天日期 取日志信息，追加写入日志文件
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);
            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year+1900, nowtm->tm_mon+1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+"); // 追加
            if (pf == nullptr)
            {
                std::cout << "logger file : " << file_name << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lockQueue.Pop();

            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s] ", 
                    nowtm->tm_hour, 
                    nowtm->tm_min, 
                    nowtm->tm_sec,
                    (m_log_level == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    writeLog.detach();
}
void Logger::SetLogLevel(LoggerLevel level)
{
    m_log_level = level;
}

// 日志 msg 写到缓冲队列
void Logger::Log(std::string msg)
{
    m_lockQueue.Push(msg);
}
