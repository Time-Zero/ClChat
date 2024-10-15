#include "groupmodel.hpp"
#include "db.hpp"
#include "group.hpp"
#include "groupuser.hpp"
#include <cstdio>
#include <cstdlib>
#include <mysql/mysql.h>
#include <strings.h>
#include <vector>

bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s','%s')"
            ,group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection().get()));
            return true;
        }
    }

    return false;
}

void GroupModel::addGroup(int userid, int groupid, std::string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser(groupid,userid,grouprole) values(%d,%d,'%s')",groupid,userid,role.c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<Group> GroupModel::queryGroup(int userid)
{
    /*
        1.首先根据userid所在groupuser表中查询出该用户所属的群组信息
        2.再根据群组信息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询，查出用户的详细信息
    */

    // 查询当前用户在哪一些群中
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc   \
                from AllGroup a inner join GroupUser b on a.id = b.groupid where b.userid = %d", userid);

    std::vector<Group> groupVec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            while(row != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);

                row = mysql_fetch_row(res);
            }

            mysql_free_result(res);
        }
    }

    // 查询当前用户所在群中有哪些人
    for(auto group : groupVec)
    {
        bzero(sql, sizeof(sql));
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from User a \
                                inner join GroupUser b on b.userid = a.id where b.groupid = %d", group.getId());

        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            while(row != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);

                row = mysql_fetch_row(res);
            }

            mysql_free_result(res);
        }
    }

    return groupVec;
}

std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d", groupid, userid);
    
    std::vector<int> idVec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            while(row != nullptr)
            {
                idVec.push_back(atoi(row[0]));
                row = mysql_fetch_row(res);
            }

            mysql_free_result(res);
        }
    }

    return idVec;
}