#include "chatservice.hpp"
#include "public.hpp"
#include <functional>
#include <muduo/base/Logging.h>

// 注册消息以及对应的回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
}

ChatService& ChatService::GetInstance()
{
    static ChatService instance;
    return instance;
}

void ChatService::login(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)
{
    LOG_INFO << "do login service!";
}

void ChatService::reg(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)
{
    LOG_INFO << "do reg service!";
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end())
    {
        // 返回一个空操作
        return [=](const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp){
            LOG_ERROR << "msgid: " << msgid << " can not find handler! ";
        };
    }
    else 
    {
        return _msgHandlerMap[msgid];
    }
}