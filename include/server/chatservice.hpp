#pragma once

#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/TcpConnection.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <unordered_map>
#include <functional>
#include <mutex>

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

    // 一对一聊天业务
    void oneChat(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp);

    // 添加好友业务
    void addFriend(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp);

    // 服务器异常，业务重置方法
    void reset();

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);

    // 处理客户端异常退出
    void clientCloseException(const muduo::net::TcpConnectionPtr& conn);
private:
    ChatService();

private:
    std::unordered_map<int , MsgHandler> _msgHandlerMap;        // 存储消息id和其对应的业务处理方法
    std::unordered_map<int, muduo::net::TcpConnectionPtr> _userConnMap;     // 存储在线用户的通信连接
    std::unordered_map<muduo::net::TcpConnectionPtr, int> _connUserMap;     // 存储通信连接和用户之间的联系，用于客户端异常断开时查找用户id
    UserModel _userModel;                                       // 用户操作
    OfflineMsgModel _offlineMsgModel;                           // 离线消息操作
    FriendModel _friendModel;
    std::mutex _mtx;                                            // _userConnMap操作的互斥锁
};