#pragma once
#include "google/protobuf/service.h"
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include<functional>
#include<iostream>
#include<google/protobuf/descriptor.h>
#include<unordered_map>
// 框架提供 专门发布rpc服务的网络对象
class RpcProvider
{
public:
    // 接收任意service Service为基类 派生出各种服务
    void NotifyService(google::protobuf::Service *service);

    // 启动RPC服务节点 提供远程网络调用服务
    void run();
private:
    // TcpServer 指针
    std::unique_ptr<muduo::net::TcpServer>m_TcpServerPtr;
    // EventLoop 对象 （epoll
    muduo::net::EventLoop m_eventLoop;

    // 服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service; // 服务对象指针
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_method_map; // 保存服务的 method
    };
    // 存储注册成功的 服务对象，服务对象的方法
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;

    // 回调函数
    void onConnection(const muduo::net::TcpConnectionPtr&);
    void onMessage(const muduo::net::TcpConnectionPtr& ptr,muduo::net::Buffer* buf,muduo::Timestamp ts);
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*); // Closure 回调函数 序列化rpc响应+网络发送
};