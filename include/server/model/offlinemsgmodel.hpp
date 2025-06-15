#ifndef OFFLINEMSGMODEL_H
#define OFFLINEMSGMODEL_H
#include <string>
#include <vector>

using namespace std;

class offlineMsgModel
{
public:
    void insert(int userid, string msg);
    void remove(int userid);
    vector<string> query(int id);
};

#endif