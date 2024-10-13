#pragma once

#include <mysql/mysql.h>
#include <string>
#include <memory>

static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "334859";
static std::string dbname = "chat";

struct MysqlConnDeleter
{
    void operator()(MYSQL* conn)
    {
        if(conn != nullptr)
        {
            mysql_close(conn);
        }
    }
};

class MySQL
{
public:
    MySQL();
    ~MySQL();

    // 获取连接
    std::shared_ptr<MYSQL> getConnection();

    // 连接数据库
    bool connect();
    
    // 更新操作
    bool update(std::string sql);

    // 查询操作
    MYSQL_RES* query(std::string sql);

private:
    std::shared_ptr<MYSQL> _conn;
};