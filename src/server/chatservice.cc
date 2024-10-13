#include "chatservice.hpp"
#include "public.hpp"
#include <cerrno>
#include <functional>
#include <muduo/base/Logging.h>
#include <mutex>
#include <nlohmann/json_fwd.hpp>
#include <usermodel.hpp>

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
    // LOG_INFO << "do login service!";
    int id = js["id"];
    std::string pwd = js["password"];

    User user = _userModel.query(id);

    nlohmann::json response;
    response["msgid"] = LOGIN_MSG_ACK;
    if(user.getId() == id && user.getPassword() == pwd)     // 如果用户存在并且密码正确
    {
        if(user.getState() == "online")                
        {
            // 用户已经登录
            response["errno"] = 2;
            response["errmsg"] = "该用户已经登录";
        }
        else
        {
            // 登录成功，记录连接信息
            {
                std::lock_guard<std::mutex> lck(_mtx);
                _userConnMap.insert({id,conn});
            }

            // 登录成功,更新用户状态信息 
            user.setState("online");
            _userModel.updateState(user);

            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
        }
    }
    else
    {
        // 用户不存在,或用户存在但是密码错误
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
    }

    conn->send(response.dump());
}

void ChatService::reg(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)
{
    // LOG_INFO << "do reg service!";
    std::string name = js["name"];
    std::string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);

    bool state = _userModel.insert(user);
    nlohmann::json response;
    response["msgid"] = REG_MSG_ACK;
    if(state)
    {
        // 注册成功
        response["errno"] = 0;
        response["id"] = user.getId();
    }
    else
    {
        // 注册失败
        response["errno"] = 1;
    }
    
    conn->send(response.dump());
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