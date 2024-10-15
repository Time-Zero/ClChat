#pragma once

#include "group.hpp"
#include <vector>

// 维护群组信息的操作接口方法
class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group &group);

    // 加入群组
    void addGroup(int userid, int groupid, std::string role);

    // 查询用户所在的群组信息
    std::vector<Group> queryGroup(int userid);

    // 根据指定groupid查询群组用户id列表，除了userid自己，用于用户群聊业务给其他用户发送消息
    std::vector<int> queryGroupUsers(int userid, int groupid);
};