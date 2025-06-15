#include "groupmodel.hpp"
#include "db.h"
#include <muduo/base/Logging.h>

bool groupModel::createGroup(Group &group)
{
    char sql[1024];
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s', '%s')",
            group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}
void groupModel::addGroup(int uid, int gid, string role)
{
    char sql[1024];
    LOG_INFO << "uid" << uid << "gid" << gid;
    sprintf(sql, "insert into GroupUser values(%d,%d,'%s')", gid, uid, role.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
    return;
}
vector<Group> groupModel::queryGrooups(int uid)
{
    char sql[1024];
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id=b.groupid where b.userid=%d", uid);

    vector<Group> groupVec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group g;
                g.setId(atoi(row[0]));
                g.setName(row[1]);
                g.setDesc(row[2]);
                groupVec.push_back(g);
            }
        }
    }

    for (Group &g : groupVec)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on a.id=b.userid where b.userid=%d", g.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                g.getUsers().push_back(user);
            }
        }
    }

    return groupVec;
}
// 查询出uid之外的gid群组中其余成员，用于群发消息
vector<int> groupModel::queryGroupUsers(int uid, int gid)
{
    char sql[1024];
    sprintf(sql, "select userid from GroupUser where groupid =%d and userid!=%d", gid, uid);

    vector<int> vid;
    MySQL mysql;
    if (mysql.connect())
    {

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vid.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vid;
}