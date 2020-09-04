#ifndef __MYSQLCLIENT_H__
#define __MYSQLCLIENT_H__

#include <string>
#include <map>
#include <stdint.h>
#include <mysql/mysql.h>

class MysqlClient
{
public:
    MysqlClient();
    ~MysqlClient();

    bool mysql_connect(const std::string& ip_port, const std::string& user, const std::string& password, const std::string& database);
    MYSQL* mysql_reconnect();
    bool mysql_exec(const std::string& sql);
    uint32_t mysql_last_affected_rows();

private:
    void init_mysql_ctx();

private:
    MYSQL* m_mysql_ctx;
};

#endif