#pragma once
#include <muduo/net/Buffer.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>

// 聊天服务器类
class ChatServer
{
public:
    // 构造函数，用于初始化聊天服务器对象
    ChatServer(muduo::net::EventLoop *loop,
                const muduo::net::InetAddress& listenAddr,
                const std::string& nameArg);
    
    // 启动服务
    void start();

private:
    // 连接回调
    void onConnection(const muduo::net::TcpConnectionPtr&);
    
    // 消息回调
    void onMessage(const muduo::net::TcpConnectionPtr&,
                            muduo::net::Buffer*,
                            muduo::Timestamp);

private:
    muduo::net::TcpServer _server;          // muduo库的服务器功能对象
    muduo::net::EventLoop *_loop;           // 事件循环指针
};