#include "friendmodel.hpp"
#include <cstdio>
#include <mysql/mysql.h>
#include <vector>
#include "db.hpp"
#include "user.hpp"

void FriendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend(userid, friendid) values(%d,%d)",userid,friendid);

    MySQL mysql;
    if(mysql.connect())
    {
        mysql.query(sql);
    }
}   

std::vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d", userid);

    std::vector<User> vec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            while(row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);

                vec.push_back(user);
                row = mysql_fetch_row(res);
            }
            mysql_free_result(res);
        }
    }

    return vec;
}