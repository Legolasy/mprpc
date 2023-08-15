#pragma once
#include<unordered_map> 
#include<string>
// kv 存储
// 框架读取配置文件类
class MprpcConfig
{
public:
    // 加载 配置文件
    void LoadConfigFile(const char* config_file);
    //查询 配置信息
    std::string Load(const std::string &key);
private:
    std::unordered_map<std::string,std::string> m_config_map;
    // 配置文件 去除空格
    void Trim(std::string &src_buf);
};