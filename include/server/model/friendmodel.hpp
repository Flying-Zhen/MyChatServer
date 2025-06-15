#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <vector>
#include "user.hpp"
using namespace std;
class friendModel
{
public:
    void insert(int userid, int friendid);
    vector<User> query(int userid);
};

#endif