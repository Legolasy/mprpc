#include "mprpcconfig.h"
#include<iostream>
#include<string>
void MprpcConfig::LoadConfigFile(const char* config_file)
{
    FILE *pf = fopen(config_file,"r");
    if(nullptr == pf)
    {
        std::cout<<config_file<<" open failed"<<std::endl;
        exit(EXIT_FAILURE);
    }   

    // 1.注释 2.正确 = 3.多余空格
    while(!feof(pf))
    {
        char buf[512] ={0};
        fgets(buf,512,pf);

        // 去掉字符串前面多余空格
        std::string src_buf(buf);
        Trim(src_buf);

        // int idx = src_buf.find_first_not_of(' '); // 找到第一个非空格下标
        // if(idx != -1)
        // {
        //     src_buf =  src_buf.substr(idx);
        // }
        // // 去掉字符串后面多余空格
        // idx = src_buf.find_last_not_of(' ');
        // if(idx != -1)
        // {
        //     src_buf =  src_buf.substr(0,idx+1);
        // }

        // 判断 # 注释
        if(src_buf[0] == '#' || src_buf.empty())
        {
            continue;
        }
        // 解析 配置项
        int idx = src_buf.find('=');
        if(idx==-1)
        {
            // 不合法
            continue;
        }

        std::string key;
        std::string value;
        key = src_buf.substr(0,idx);
        Trim(key);
        int end_idx =  src_buf.find('\n'); // 换行符
        value = src_buf.substr(idx+1,end_idx-idx-1);
        Trim(value);
        m_config_map.insert({key,value});
    }
}

std::string MprpcConfig::Load(const std::string& key)
{
    // 不可以！ 如果key不存在，返回会增加这个key的键值对
    //return m_config_map[key]; 
    if(m_config_map.count(key)!=0)
        return m_config_map[key];
    return "";
}
    
// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串前面有空格
        src_buf = src_buf.substr(idx, src_buf.size()-idx);
    }
    // 去掉字符串后面多余的空格
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串后面有空格
        src_buf = src_buf.substr(0, idx+1);
    }
}