#include "chatserver.hpp"
#include "chatservice.hpp"
#include <muduo/net/TcpServer.h>
#include <functional>
#include <nlohmann/json_fwd.hpp>
#include <thread>
#include <string>
#include <nlohmann/json.hpp>

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
    // 用户断开连接
    if(!conn->connected())
    {
        ChatService::GetInstance().clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buf,
                            muduo::Timestamp time_stamp)
{
    std::string buffer = buf->retrieveAllAsString();
    nlohmann::json js = nlohmann::json::parse(buffer);      // 消息的反序列化

    // 将网络模块和业务模块解耦
    // 通过js["msgid"]获取对应的handler
    auto msgHandler =  ChatService::GetInstance().getHandler(js["msgid"].get<int>());
    // 回调消息对应的绑定好的事件处理器来进行对应的事件处理
    msgHandler(conn,js,time_stamp);
}