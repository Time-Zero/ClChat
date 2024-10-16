#include "chatservice.hpp"
#include "public.hpp"
#include "user.hpp"
#include <functional>
#include <muduo/base/Logging.h>
#include <mutex>
#include <nlohmann/json_fwd.hpp>
#include <usermodel.hpp>
#include <vector>
#include <string>
#include "usermodel.hpp"
#include "group.hpp"

// 注册消息以及对应的回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    if(_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubscribeMessage,this,std::placeholders::_1,std::placeholders::_2));
    }
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
            response["errmsg"] = "this account is using, input another!";
        }
        else
        {
            // 登录成功，记录连接信息
            {
                std::lock_guard<std::mutex> lck(_mtx);
                _userConnMap.insert({id,conn});
                _connUserMap.insert({conn,id});
            }

            // 用户登录成功后向用户订阅channel
            _redis.subscribe(user.getId());

            // 登录成功,更新用户状态信息 
            user.setState("online");
            _userModel.updateState(user);

            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询该用户是否有离线消息
            std::vector<std::string> vec = _offlineMsgModel.query(user.getId());
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 用户上线读取离线消息后，删除离线消息
                _offlineMsgModel.remove(user.getId());
            }

            std::vector<User> user_vec = _friendModel.query(user.getId());
            if(!user_vec.empty())
            {
                std::vector<std::string> temp_vec;
                for(auto it : user_vec)
                {
                    nlohmann::json js;
                    js["id"] = it.getId();
                    js["name"] = it.getName();
                    js["state"] = it.getState();
                    temp_vec.push_back(js.dump());
                }
                response["friends"] = temp_vec;
            }
        }
    }
    else
    {
        // 用户不存在,或用户存在但是密码错误
        response["errno"] = 1;
        response["errmsg"] = "username or password error!";
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

void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr& conn)
{
    std::unique_lock<std::mutex> lck(_mtx);
    // 查找对应的连接
    auto it = _connUserMap.find(conn);
    // 如果连接存在
    if(it != _connUserMap.end())
    {
        // 找到用户id
        int id = it->second;

        // 从用户连接表中查找用户
        auto user_it = _userConnMap.find(id);
        // 如果找不到，直接返回
        if(user_it == _userConnMap.end())       return; 
        _userConnMap.erase(user_it);

        // 从连接用户表中删除连接
        _connUserMap.erase(it);

        // 完成映射表的操作，可以释放锁了
        lck.unlock();
    
        // 更新用户的状态
        User user;
        user.setId(id);
        user.setState("offline");
        _userModel.updateState(user);
        _redis.unsubscribe(id);
    }
}

void ChatService::oneChat(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)
{
    int toid = js["to"].get<int>();
    
    {
        std::lock_guard<std::mutex> lck(_mtx);
        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end())
        {
            it->second->send(js.dump());
            return;
        }
    }

    User user = _userModel.query(toid);
    if(user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    _offlineMsgModel.insert(toid, js.dump());
}

void ChatService::reset()
{
    _userModel.resetState();
}


void ChatService::addFriend(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)
{
    int userid = js["id"];
    int friendid = js["friendid"];

    // 存储好友信息
    _friendModel.insert(userid, friendid);
}

void ChatService::createGroup(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)
{
    int userid = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];

    Group group(-1,name,desc);
    if(_groupModel.createGroup(group))      // 创建群组，并把创建者设置为'creator'身份
    {
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)
{
    int userid = js["id"];
    int groupid = js["groupid"];
    _groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)
{
    int userid = js["id"];
    int groupid = js["groupid"];
    std::vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    for(int id : useridVec)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end())
        {
            // 在线消息转发
            it->second->send(js.dump());
        }
        else
        {
            User user = _userModel.query(id);
            if(user.getState() == "online"){
                _redis.publish(id, js.dump());
            }
            else
            {
                // 离线消息保存
                _offlineMsgModel.insert(id, js.dump());
            }

        }
    }
}

void ChatService::loginout(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp)
{
    int userid = js["id"].get<int>();

    {
        std::lock_guard<std::mutex> lck(_mtx);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end())
        {
            _connUserMap.erase(it->second);
            _userConnMap.erase(it);
        }
    }
    
    //  redis注销，相当于下线了，在redis中取消订阅通道
    _redis.unsubscribe(userid);

    User user(userid,"","","offline");
    _userModel.updateState(user);
}

void ChatService::handlerRedisSubscribeMessage(int userid, std::string msg)
{
    std::lock_guard<std::mutex> lck(_mtx);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    _offlineMsgModel.insert(userid, msg);
}