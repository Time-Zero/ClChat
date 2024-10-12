#include "chatserver.hpp"
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
using namespace std;

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1", 6000);

    ChatServer chat_server(&loop, addr, "ChatServer");
    chat_server.start();
    loop.loop();        // 开启事件循环

    return 0;
}