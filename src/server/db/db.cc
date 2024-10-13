#include "db.hpp"
#include <mysql/mysql.h>
#include <muduo/base/Logging.h>

extern std::string server;
extern std::string user;
extern std::string password;
extern std::string dbname;

MySQL::MySQL() : _conn(mysql_init(nullptr), MysqlConnDeleter())
{

}

MySQL::~MySQL()
{
    _conn.reset();
}

bool MySQL::connect()
{
    MYSQL* p = mysql_real_connect(_conn.get(), server.c_str(), user.c_str(), 
                            password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if(p != nullptr)
    {
        mysql_query(_conn.get(), "set names gbk");              // 让代码能够支持中文
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
    }
    
    return p;
}

bool MySQL::update(std::string sql)
{
    if(!mysql_query(_conn.get(), sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
        return false;
    }
    
    return true;
}

MYSQL_RES* MySQL::query(std::string sql)
{
    if(!mysql_query(_conn.get(), sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败!";
        return nullptr;
    }

    return mysql_use_result(_conn.get());
}

std::shared_ptr<MYSQL> MySQL::getConnection()
{
    return this->_conn;
}