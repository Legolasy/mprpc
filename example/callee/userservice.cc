#include<iostream>
#include<string>
#include "../user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
//using namespace fixbug;


// 展示如何将本地方法 to RPC 方法
// 1.写一个protobuf文件，说明message，service
// 2.实现框架下的rpc函数，接收到request后获取数据，进行本地处理，然后填充到response，并调用回调函数
class UserService : public fixbug::UserServiceRPC // RPC服务 提供方
{
public:
    bool Login(std::string name,std::string pwd)
    {
        std::cout<<"Local Login Method Called"<<std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;  
        return true;
    }    

    // 重写基类虚函数
    // caller ---> Login(LoginRequest) ---> muduo -> callee
    // callee ---> Login(LoginRequest) ---> 交到下面充血的Login方法
    void Login(google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 框架给业务上报了请求参数 LoginRequest, 应用 获取相应参数，进行业务处理
        // 参数在 LoginRequest 中保存
        std::string name = request->name();
        std::string pwd = request->pwd();


        bool login_rs = Login(name,pwd); // 本地业务完成

        // 把响应写入框架 response即可
        fixbug::ResultCode *rc = response->mutable_result();
        rc->set_errcode(0);
        rc->set_errmsg("");
        response->set_success(login_rs);
        
        // 调用回调函数 执行响应对象的数据序列化+网络发送，由框架完成
        done->Run();
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout<<"Local Register Method Called"<<std::endl;
        std::cout << "id:" << id << " name:" << name << std::endl;  
        return true;
    }
    void Register(google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
                       {
                            // 获取请求参数
                            uint32_t id =request->id();
                            std::string name = request->name();
                            std::string pwd = request->pwd();

                            // 进行本地业务
                            bool rs = Register(id,name,pwd);

                            // 结果 填写到 response 别忘了message response中的其他参数
                            response->mutable_result()->set_errcode(0);
                            response->mutable_result()->set_errmsg("");
                            response->set_success(rs);

                            // 回调 response 执行序列化+网络发送
                            done->Run();
                       }
};

int main(int argc,char** argv)
{
    // 调用框架 初始化
    MprpcApplication::Init(argc,argv);

    // 把UserService对象 发布到RPC节点上 provider是一个rpc网络服务对象
    RpcProvider provider; 
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点 run后阻塞 等待远程rpc调用
    provider.run();
}