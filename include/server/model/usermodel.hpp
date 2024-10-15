#pragma once

#include "user.hpp"

// User表的数据操作类
class UserModel
{
public:
    // User表的增加方法
    bool insert(User &user);
    
    // 根据用户号码，查询用户信息
    User query(int id);

    bool updateState(User user);

    // 将所有用户的状态重置为offline
    void resetState();
private:

};