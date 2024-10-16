#include "redis.hpp"
#include <cstdlib>
#include <functional>
#include <hiredis/hiredis.h>
#include <hiredis/read.h>
#include <iostream>
#include <pthread.h>
#include <string>
#include <thread>

Redis::Redis() : _puhlish_context(nullptr), _subscribe_context(nullptr)
{

}

Redis::~Redis()
{
    if(_puhlish_context != nullptr)
    {
        redisFree(_puhlish_context);
    }

    if(_subscribe_context != nullptr)
    {
        redisFree(_subscribe_context);
    }
}

bool Redis::connect()
{
    _puhlish_context = redisConnect("127.0.0.1", 6379);
    if(_puhlish_context == nullptr)
    {
        std::cerr << "connect redis failed!"  << std::endl;
        return false;
    }

    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if(_subscribe_context == nullptr)
    {
        std::cerr << "connect redis failed!" << std::endl;
        return false;
    }

    std::thread t([&](){
        observer_channel_message();
    });
    t.detach();

    std::cout << "connect redis-server success !" << std::endl;
    return true;
}

bool Redis::publish(int channel, std::string message)
{
    redisReply* reply = (redisReply*)redisCommand(_puhlish_context, "PUBLISH %d %s", channel, message.c_str());
    if(reply == nullptr)
    {
        std::cerr << "push command failed!" << std::endl;
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    // subscribe命令本身会造成线程阻塞等待通道中的消息，所以这里只做订阅消息，不接受通道消息
    // 通道消息的接收专门在observer_channel_message函数中进行
    // 只负责发送命令，不阻塞接收redis server的相应消息，否则和notifyMSG线程抢占资源
    if(redisAppendCommand(_subscribe_context, "SUBSCRIBE %d", channel) == REDIS_ERR)
    {
        std::cerr << "subscribe command failed!" << std::endl;
        return false;
    }

    // redisBufferWrite可以循环发送缓冲区，知道缓冲区数据发送完毕 (done 被置为1)
    int done = 0;
    while(!done)
    {
        if(redisBufferWrite(_subscribe_context, &done))
        {
            std::cerr << "subscribe command failed!" << std::endl;
            return false;
        }
    }

    // redisCommand = redisAppendCommand(把命令放到本地缓冲区) + redisBufferWrite(把本地缓冲区中的命令发送到redis) + redisReply(等待redis相应)
    // 但是我们subscribe不会立即返回消息，而是会阻塞等待消息。可我们实际情况是不需要阻塞等待消息的，我们只需要告诉redis我们订阅这个消息，所以上面的逻辑没有Reply的过程

    return true;
}

bool Redis::unsubscribe(int channel)
{
    if(redisAppendCommand(_subscribe_context, "UNSUBSCRIBE %d", channel) == REDIS_ERR)
    {
        std::cerr << "unsubscribe command failed!" << std::endl;
        return false;
    }

    // redisBufferWrite可以循环发送缓冲区，知道缓冲区数据发送完毕 (done 被置为1)
    int done = 0;
    while(!done)
    {
        if(redisBufferWrite(_subscribe_context, &done))
        {
            std::cerr << "unsubscribe command failed!" << std::endl;
            return false;
        }
    }

    return true;
}

void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    while (redisGetReply(_subscribe_context, (void**)&reply) == REDIS_OK) 
    {   
        // 订阅收到的消息是一个三元组
        if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道中发生的消息
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    std::cerr << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
}

void Redis::init_notify_handler(std::function<void(int, std::string)> fn)
{
    this->_notify_message_handler = fn;
}