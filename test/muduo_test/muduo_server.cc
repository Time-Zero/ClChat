#include <muduo/base/Timestamp.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>
#include <functional>
#include <thread>
#include <iostream>

/*基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数的参数，编写CharServer的构造函数
4.在当前服务器类的构造函数中，注册处理连接的回调函数和注册处理读写事件的回调函数
5.设置合适的服务端线程数量,muduo库会自己分配io线程和worker线程
*/ 

class ChatServer
{
public:
    ChatServer(muduo::net::EventLoop* loop,                     // 事件循环 
                const muduo::net::InetAddress& listenAddr,      // ip+port
                const std::string& nameArg)                          // 服务器的名称
                : _server(loop, listenAddr, nameArg),
                _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this, std::placeholders::_1));

        // 给服务器注册用户读写事件的回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // 设置服务器端的线程数，执行线程池大小为cpu线程数，分配情况是一个IO线程，n-1个worker线程
        _server.setThreadNum(std::thread::hardware_concurrency());
    }

    // 开启事件循环
    void start()
    {
        _server.start();
    }

private:
    // 用户连接的创建和断开回调函数
    void onConnection(const muduo::net::TcpConnectionPtr& conn)
    {
        if(conn->connected())
        {
            std::cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << " State:On " << std::endl; 
        }
        else 
        {
            std::cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << " State:Offline " << std::endl; 
            conn->shutdown();
        }
    }

    // 专门处理用户的读写事件
    void onMessage(const muduo::net::TcpConnectionPtr& conn,            // 连接
                    muduo::net::Buffer *buf,                            // 缓冲区
                    muduo::Timestamp time)                              // 接收到数据的事件信息
    {
        std::string buffer = buf->retrieveAllAsString();
        std::cout << "recv buffer:" << buffer << " time:" << time.toString() << std::endl;
        conn->send(buffer);
    }

private:
    muduo::net::TcpServer _server;
    muduo::net::EventLoop *_loop;
};

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();            // 就等于epoll_wait，以阻塞的方式等待新用户连接，已连接用户的读写事件

    return 0;
}