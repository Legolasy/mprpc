#include "mprpcapplication.h"
#include<iostream>
#include "user.pb.h"
#include "mprpcchannel.h"
int main(int argc,char** argv)
{
    // 初始化mprpc框架 一次就可以
    MprpcApplication::Init(argc,argv);

    // 远程调用login
    fixbug::UserServiceRPC_Stub stub(new MprpcChannel()); //RpcChannel负责序列化
    fixbug::LoginRequest request;
    request.set_name("abc");
    request.set_pwd("123");
    fixbug::LoginResponse response;
    // 发起一次rpc调用 同步阻塞
    stub.Login(nullptr,&request,&response,nullptr); // RpcChannel ->callMethod

    // 一次rpc调用完成
    if(0 == response.result().errcode())
    {
        std::cout << "rpc login response success:" << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
    }
    // 演示rpc调用

    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("abc");
    req.set_pwd("123");
    fixbug::RegisterResponse rsp;

    //同步方式 等待返回结果
    stub.Register(nullptr,&req,&rsp,nullptr);
    if(0 == rsp.result().errcode())
    {
        std::cout << "rpc Register response success:" << rsp.success() << std::endl;
    }
    else
    {
        std::cout << "rpc Register response error : " << rsp.result().errmsg() << std::endl;
    }
    return 0;
}