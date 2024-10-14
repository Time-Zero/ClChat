#pragma once

#include "groupuser.hpp"
#include <string>
#include <vector>


// Group的ORM类
class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc="")
    {
        this->_id = id;
        this->_name = name;
        this->_desc = desc;
    }

    void setId(int id)
    {
        this->_id = id;
    }

    void setName(std::string name)
    {
        this->_name = name;
    }

    void setDesc(std::string desc)
    {
        this->_desc = desc;
    }

    int getId()
    {
        return this->_id;
    }

    std::string getName()
    {
        return this->_name;
    }

    std::string getDesc()
    {
        return this->_desc;
    }

    std::vector<GroupUser>& getUsers()
    {
        return this->_users;
    }

private:
    int _id;
    std::string _name;
    std::string _desc;
    std::vector<GroupUser> _users;
};