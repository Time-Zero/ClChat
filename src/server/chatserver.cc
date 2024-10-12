#include "chatserver.hpp"
#include <muduo/net/TcpServer.h>
#include <functional>
#include <thread>

ChatServer::ChatServer(muduo::net::EventLoop *loop,
                const muduo::net::InetAddress& listenAddr,
                const std::string& nameArg) 
                : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,std::placeholders::_1));
    
    // 注册消息
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    // 设置server服务线程
    _server.setThreadNum(std::thread::hardware_concurrency());
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{

}

void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buf,
                            muduo::Timestamp time_stamp)
{

}