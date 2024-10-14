#include "chatserver.hpp"
#include "chatservice.hpp"
#include <csignal>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <signal.h>

// 处理服务器ctrl+c退出后，重置user状态信息
void resetHandler(int)
{
    ChatService::GetInstance().reset();
    exit(0);
}

using namespace std;

int main()
{
    std::signal(SIGINT,resetHandler);

    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1", 6000);

    ChatServer chat_server(&loop, addr, "ChatServer");
    chat_server.start();
    loop.loop();        // 开启事件循环

    return 0;
}