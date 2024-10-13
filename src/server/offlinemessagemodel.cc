#include "offlinemessagemodel.hpp"
#include "db.hpp"
#include <mysql/mysql.h>
#include <string>
#include <vector>

void OfflineMsgModel::insert(int userid, std::string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage(userid, message) values(%d,'%s') ", userid, msg.c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

void OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid = %d", userid);

    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<std::string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);

    std::vector<std::string> vec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            while(row != nullptr)
            {
                vec.push_back(row[0]);
                row = mysql_fetch_row(res);
            }

            mysql_free_result(res);
        }
    }

    return vec;
}