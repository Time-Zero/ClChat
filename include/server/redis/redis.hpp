#pragma once
#include <functional>
#include <string>
#include <hiredis/hiredis.h>

class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器
    bool connect();

    // 向redis指定通道发布消息
    bool publish(int channel, std::string message);

    // 向redis指定通道subscribe订阅消息
    bool subscribe(int channel);

    // 向redis指定通道unsubscribe取消订阅消息
    bool unsubscribe(int channel);

    // 在独立线程中接收订阅通道中的消息
    void observer_channel_message();

    // 初始化向业务层上报通信消息的回调对象
    void init_notify_handler(std::function<void(int, std::string)> fn);

private:
    // hiredis同步上下文对象，服务pushlish消息
    redisContext* _puhlish_context;
    
    // hiredis同步上下文对象，负责subsrcibe消息
    redisContext* _subscribe_context;

    // 回调操作，收到订阅的消息，向service层上报
    std::function<void(int, std::string)> _notify_message_handler;
};