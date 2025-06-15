#include "offlinemsgmodel.hpp"
#include "db.h"

void offlineMsgModel::insert(int userid, string msg)
{
    char sql[1024];
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
    return;
}
void offlineMsgModel::remove(int userid)
{
    char sql[1024];
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
    return;
}
vector<string> offlineMsgModel::query(int userid)
{
    vector<string> vec;
    char sql[1024];
    sprintf(sql, "select * from OfflineMessage where userid = %d", userid);

    MySQL mysql;

    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);

        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[1]);
            }
        }
        mysql_free_result(res);
    }
    return vec;
}