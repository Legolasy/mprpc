#pragma once


#include "mprpcconfig.h"
// mprpc 框架初始化类 单例模式设计 共享信息
class MprpcApplication
{
public:
    static void Init(int argc,char** argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetMprpcConfig();

private:
    MprpcApplication(){};
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;

    static MprpcConfig m_config; // 负责加载配置文件
};