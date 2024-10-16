#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <nlohmann/json.hpp>
#include <iostream>
#include <nlohmann/json_fwd.hpp>
#include <thread>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "db.hpp"
#include "group.hpp"
#include "groupuser.hpp"
#include "user.hpp"
#include "public.hpp"

using namespace std;

// 记录当前用户的登录信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
std::vector<User> g_currentUserFriendList;
// 记录当前用户的群组列表信息
std::vector<Group> g_currentUserGroupList;
// 显示当前登录用户的基本信息
void showCurrentUserData();

// 接收线程
void readTaskHandler(int clientfd);
// 获取系统事件(聊天信息需要添加时间信息)
std::string getCurrentTime();
// 主聊天页面程序
void mainMenu(int clientfd);
// 帮助界面
void help();

// 聊天客户端实现，main线程负责发送，子线程负责接收
int main(int argc, char** argv)
{
    if(argc < 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if(connect(clientfd, (sockaddr*)(&server), sizeof(sockaddr_in)) == -1)
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    for(;;)
    {
        cout << "======================================" << endl;
        cout << "1.login" << endl;
        cout << "2.register" << endl;
        cout << "3.quit" << endl;
        cout << "======================================" << endl;
        cout << "choice: ";
        int choice = 0;
        cin >> choice;
        cin.get();          // 清空缓冲区回车

        switch (choice) {
            case 1:         // login
            {
                int id = 0;
                char pwd[50] = {0};
                cout << "userid:";
                cin >> id;
                cin.get();
                cout << "password:";
                cin.getline(pwd,50);

                nlohmann::json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                std::string request = js.dump();

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1,0);
                if(len == -1)
                {
                    cerr << "send login msg error: "<< request << endl;
                }
                else
                {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if(len == -1)
                    {
                        cerr << "recv login response error" << endl;
                    }
                    else
                    {
                        nlohmann::json responsejs = nlohmann::json::parse(buffer);
                        if(responsejs["errno"].get<int>() != 0)         // 登录失败
                        {
                            cerr << responsejs["errmsg"] << endl;
                        }
                        else    // 登录成功
                        {
                            g_currentUser.setId(responsejs["id"].get<int>());
                            g_currentUser.setName(responsejs["name"]);

                            if(responsejs.contains("friends"))
                            {
                                vector<std::string> vec = responsejs["friends"];
                                for(std::string& str : vec)
                                {
                                    nlohmann::json js = nlohmann::json::parse(str);
                                    User user;
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    g_currentUserFriendList.push_back(user);
                                }
                            }

                            if(responsejs.contains("groups"))
                            {
                                vector<std::string> vec1 = responsejs["groups"];
                                for(auto grpstr : vec1)
                                {
                                    nlohmann::json grpjs = nlohmann::json::parse(grpstr);
                                    Group group;
                                    group.setId(grpjs["id"].get<int>());
                                    group.setName(grpjs["groupname"]);
                                    group.setDesc(grpjs["groupdesc"]);

                                    vector<std::string> vec2 = grpjs["users"];
                                    for(auto userstr : vec2)
                                    {
                                        GroupUser user;
                                        nlohmann::json js = nlohmann::json::parse(userstr);
                                        user.setId(js["id"].get<int>());
                                        user.setName(js["name"]);
                                        user.setState(js["state"]);
                                        user.setRole(js["role"]);
                                        group.getUsers().push_back(user);
                                    }

                                    g_currentUserGroupList.push_back(group);
                                }
                            }

                            showCurrentUserData();

                            if(responsejs.contains("offlinemsg"))
                            {
                                vector<std::string> vec = responsejs["offlinemsg"];
                                for(auto str : vec)
                                {
                                    nlohmann::json js = nlohmann::json::parse(str);
                                    // time + [id] + name + "said: " + xxxx
                                    cout << js["time"] << "[" << js["id"] << "]" << js["name"] << " said: " << js["msg"] << endl;
                                }
                            }

                            std::thread readTask(readTaskHandler,clientfd);
                            readTask.detach();
                            mainMenu(clientfd);
                        }
                    }
                }

                break;
            }
            case 2:         // register
            {
                char name[50] = {0};
                char pwd[50] = {0};
                cout << "username: ";
                cin.getline(name,50);
                cout << "password: ";
                cin.getline(pwd,50);

                nlohmann::json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                std::string request = js.dump();
                cout << request << endl;

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if(len == -1)
                {
                    cerr << "send reg msg error: " << request << endl;
                }
                else
                {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if(len == -1)
                    {
                        cerr << "recv reg response error" << endl;
                    }
                    else
                    {
                        nlohmann::json responsejs = nlohmann::json::parse(buffer);
                        if(responsejs["errno"].get<int>() != 0)
                        {
                            cerr << name << " is already exist, register error!" << endl;
                        }
                        else
                        {
                            cout << name << " register success, userid is " << responsejs["id"] << ", do not forget it!" << endl;
                        }
                    }
                }


                break;
            }
            case 3:         // quit 业务
            {
                close(clientfd);
                exit(0);
                break;
            }
            default:
            {
                cerr << "invalid input! please input again: " << endl;
                break;
            }
        }
    }

}

void showCurrentUserData()
{
    cout << "=======================================loing user====================================" << endl;
    cout << "current login user => id: " << g_currentUser.getId() << " name: " << g_currentUser.getName() << endl;
    cout << "---------------------------------------friend list---------------------------- -------" << endl;

    if(!g_currentUserFriendList.empty())
    {
        for(auto user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------------------------group list------------------------------------" << endl;
    
    if(!g_currentUserGroupList.empty())
    {
        for(auto group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for(auto user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << endl;
            }
        }
    }
    cout << "=====================================================================================" << endl;
}

void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if(len == -1 || len == 0)
        {
            close(clientfd);
            exit(-1);
        }

        // 接收chatserver转发的数据，反序列化生成json数据对象
        nlohmann::json js = nlohmann::json::parse(buffer);
        if(js["msgid"].get<int>() == ONE_CHAT_MSG)
        {
            cout << js["time"].get<std::string>() << "[" << js["id"] << "]" << js["name"] << " said: " << js["msg"] << endl;
            continue;
        }
    }
}

std::string getCurrentTime()
{
    return std::string();
}


std::unordered_map<std::string, std::function<void(int,std::string)>> commandHandlerMap = {
    // {"help", help},
    // {"char", chat},
    // {"addfriend", addfriend}
    // {"creategroup",creategroup}
    // {"addgroup",addgroup}
    // {"groupchat",groupchat}
    // {"quit",quit}
};

void mainMenu(int clientfd)
{
    help();
}

void help()
{
    
}