#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <vector>
#include <string>

class groupModel
{
public:
    bool createGroup(Group &group);
    void addGroup(int uid, int gid, string role);
    vector<Group> queryGrooups(int uid);
    // 查询出uid之外的gid群组中其余成员，用于群发消息
    vector<int> queryGroupUsers(int uid, int gid);
};
#endif