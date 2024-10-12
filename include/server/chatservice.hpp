#pragma once

#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/TcpConnection.h>
#include <nlohmann/json_fwd.hpp>
#include <unordered_map>
#include <functional>

// 处理消息的回调方法类型
using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)>;

// 聊天服务器业务类
class ChatService
{
public:
    ChatService(const ChatService&) = delete;
    static ChatService& GetInstance();
    
    // 处理登录业务
    void login(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp);
    
    // 处理注册业务
    void reg(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp);

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
private:
    ChatService();

private:
    std::unordered_map<int , MsgHandler> _msgHandlerMap;        // 存储消息id和其对应的业务处理方法
};