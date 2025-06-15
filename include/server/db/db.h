#ifndef DB_H
#define DB_H
#include <string>
#include <mysql/mysql.h>
using namespace std;

class MySQL
{
public:
    MySQL();
    ~MySQL();
    bool connect();
    bool update(string sql);
    MYSQL_RES *query(string sql);
    MYSQL *getConnection();

private:
    MYSQL *_conn;
};

#endif