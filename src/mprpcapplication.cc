#include "mprpcapplication.h"
#include<iostream>
#include<unistd.h>
#include<string>


MprpcConfig MprpcApplication::m_config;

void showArgsHelp()
{
    std::cout<<"format: command -i <config_file>"<<std::endl;
}
void MprpcApplication::Init(int argc, char** argv) 
{
    if(argc<2)
    {
        showArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while((c=getopt(argc,argv,"i:"))!=-1)
    {
        switch (c)
        {
            case 'i':  // 读取配置文件
                config_file = optarg; 
                break;
            case '?':
                std::cout<<"Invalid Args!"<<std::endl;
                showArgsHelp();
                exit(EXIT_FAILURE);
            case ':':
                std::cout<<"Need <config_file>"<<std::endl;
                showArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    // 加载配置
    m_config.LoadConfigFile(config_file.c_str());

    //std::cout<<"rpc server ip is:"<<m_config.Load("rpcserverip")<<std::endl;
}

MprpcApplication& MprpcApplication::GetInstance()
{
        static MprpcApplication app;
        return app;
}

MprpcConfig& MprpcApplication::GetMprpcConfig() {
        // TODO: 在此处插入 return 语句
        return m_config;
}
