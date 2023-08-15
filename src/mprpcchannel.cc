#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include<string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include"zookeeperutil.h"
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                              google::protobuf::RpcController* controller,
                              const google::protobuf::Message* request,
                              google::protobuf::Message* response,
                              google::protobuf::Closure* done) 
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name  = method->name();

    // 获取参数的序列化字符串长度 args_size
    int args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        std::cout<<"Serialize Request Failed\n";
        controller->SetFailed("serialize request error!");
        return ;
    }

    // 定义rpc请求头
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    // 序列化为字符串
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        std::cout<<"Serialize Rpc Header Failed\n";
        controller->SetFailed("serialize rpc header error!");
        return ;
    }

    // 组织待发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char*)&header_size,4)); // 4字节 保存header size
    send_rpc_str +=rpc_header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_str: " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;

    // socket 发送rpc调用
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == clientfd)
    {
        char errtxt[512] = {0};
        std::cout<<"create socket errno: "<<errno<<std::endl;
        sprintf(errtxt, "create socket error! errno:%d", errno);
        controller->SetFailed(errtxt);
        exit(EXIT_FAILURE);
    }

    // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str()); 
    // std::string ip = MprpcApplication::GetInstance().GetMprpcConfig().Load("rpcserverip");
    // uint16_t port = stoi(MprpcApplication::GetInstance().GetMprpcConfig().Load("rpcserverport"));

    struct sockaddr_in server_addr;
    server_addr.sin_family =AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 链接rpc服务节点
    if(-1 == connect(clientfd,(struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        std::cout<<"connect errno: "<<errno<<std::endl;
        close(clientfd);
        exit(EXIT_FAILURE);
    }

    // 发送rpc请求
    if(-1 == send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0))
    {
        // 发送失败
        close(clientfd);
        std::cout<<"send errno: "<<errno<<std::endl;
    }

    // 接收rpc请求
    char recv_buf[1024]={0};
    int recv_size = 0;
    if(-1 == (recv_size = recv(clientfd,recv_buf,1024,0)))
    { 
        // 接收失败
        std::cout<<"recv errno: "<<errno<<std::endl;
        close(clientfd);
        return ;
    }

    // 结果写回response
    std::string response_str(recv_buf,0,recv_size);

    // 数据反序列化
    // bug: recv_buf遇到\0 后面的数据无法保存，反序列化失败
    //if(!response->ParseFromString(response_str)) 
    if(!response->ParseFromArray(recv_buf,recv_size))
    {
        // 接收失败
        std::cout<<"parse response error  "<<response_str<<std::endl;
        close(clientfd);
        return ;
    }

    close(clientfd);
}
