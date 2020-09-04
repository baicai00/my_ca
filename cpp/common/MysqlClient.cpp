#include "MysqlClient.h"


MysqlClient::MysqlClient()
{
}

MysqlClient::~MysqlClient()
{
}

bool MysqlClient::mysql_connect(const std::string& ip_port, const std::string& user, const std::string& password, const std::string& database)
{
}

MYSQL* MysqlClient::mysql_reconnect()
{
}

bool MysqlClient::mysql_exec(const std::string& sql)
{
}

uint32_t MysqlClient::mysql_last_affected_rows()
{
}

void MysqlClient::init_mysql_ctx()
{
}


