#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include"zookeeperutil.h"
// 框架提供的 发布rpc方法的接口
void RpcProvider::NotifyService(google::protobuf::Service* service) 
{
    ServiceInfo service_info;

    // 获取 Service 的描述信息
    const google::protobuf::ServiceDescriptor *pServiceDesc = service->GetDescriptor();
    // 获取 Service 名字
    std::string service_name = pServiceDesc->name();
    // 获取 Service 方法数量
    int method_cnt = pServiceDesc->method_count();

    std::cout<<"service_name: "<<service_name<<std::endl;

    for(int i=0;i<method_cnt;i++)
    {
        // 获取Service 指定下标的 Method方法的描述
        auto pmethodDesc = pServiceDesc->method(i);
        auto method_name = pmethodDesc->name();
        service_info.m_method_map.insert({method_name,pmethodDesc}); // 方法名-方法描述指针
        std::cout<<"method_name: "<<method_name<<std::endl;
    }

    service_info.m_service = service;
    m_serviceMap.insert({service_name,service_info}); // 服务名 - 描述服务 的 结构体
}
// 启动rpc服务节点 开始提供远程rpc网络调用服务
// muduo库 分离网络层和业务
void RpcProvider::run() 
{
    std::string ip = MprpcApplication::GetInstance().GetMprpcConfig().Load("rpcserverip");
    uint16_t port = std::stoi( MprpcApplication::GetInstance().GetMprpcConfig().Load("rpcserverport"));
    muduo::net::InetAddress address(ip,port);

    // 创建 tcp server
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    // 绑定 回调函数 onConnection onMessage
    server.setMessageCallback(std::bind(&RpcProvider::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection,this,std::placeholders::_1));
    // 设置 muduo库线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap) 
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_method_map)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }


    // 启动 网络服务 开始监听 one loop per thread 
    std::cout<<"RPC Server Start at ip:"<<ip<<" port:"<<port<<std::endl;
    server.start();
    m_eventLoop.loop();
    
}

void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr& conn) 
{
    if(!conn->connected())
    {
        // rpc client 连接断开
        conn->shutdown();
    }
}


/*
rpc clent 和 rpc server 需要 一致的protobuf 数据类型

service_name method_name args

UserServiceLoginzhang san 123 需要数据拆分

header_size + header_str + args_str

粘包问题：记录参数长度解决
*/
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buf, muduo::Timestamp ts)
{
    // 反序列化  调用函数 获得结果 序列化发回去
    std::string recv_buf = buf ->retrieveAllAsString();

    // 字符流 前四个字节 保存 header_size
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size,4,0);

    // 根据header_size 读取 原始字符流 反序列化-> rpc请求 详细信息
    std::string rpc_header_str = recv_buf.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;

    std::string service_name;
    std::string method_name;
    uint32_t args_size;

    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头 反序列化 成功 
        // 获取数据头数据
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据头 反序列化 失败
        std::cout<<"rpc_header_str:"<<rpc_header_str<<" parse error"<<std::endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);
    
    // 调试
    std::cout<<"======================="<<std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_str: " << args_str << std::endl; 
    std::cout<<"======================="<<std::endl;

    // 获取 service 对象 method 对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end())
    {
        // service 不存在
        std::cout<<"Service Not Existed!"<<std::endl;
        return ;
    }

    auto method_it = it->second.m_method_map.find(method_name);
    if(method_it==it->second.m_method_map.end())
    {
        // service 不存在
        std::cout<<"Method Not Existed!"<<std::endl;
        return ;
    }

    google::protobuf::Service *service = it->second.m_service; // 获取service对象 UserService
    const google::protobuf::MethodDescriptor * method = method_it->second; // 获取 method对象 Login

    // 生成rpc方法调用的请求request 响应response 参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    if(!request->ParseFromString(args_str)) 
    {
        std::cout<<"Request Parse Error"<<args_str <<std::endl;
        return;
    }   

    // 给下面的CallMethod 创建一个 Closure回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const muduo::net::TcpConnectionPtr&, 
                                                                    google::protobuf::Message*>
                                                                    (this, 
                                                                    &RpcProvider::SendRpcResponse, 
                                                                    conn, response);

    // 在框架上，根据远端的rpc请求，调用本地方法
    service->CallMethod(method,nullptr,request,response,done);

}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn,
                                  google::protobuf::Message *response) 
{
    //序列化response结果 发送
    std::string response_str;
    if(response->SerializeToString(&response_str))
    {
        conn->send(response_str);
        conn->shutdown(); // http 短链接 主动断开链接，提供更多服务
    }
    else
    {
        std::cout<<"Serialize Response Str Failed\n";
    }
    conn->shutdown();
}
