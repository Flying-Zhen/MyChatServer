#ifndef USER_H
#define USER_H
#include <string>
using namespace std;
class User
{
public:
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    void setId(int id)
    {
        this->id = id;
    }

    void setName(string name)
    {
        this->name = name;
    }

    void setPwd(string pwd)
    {
        this->password = pwd;
    }

    void setState(string st)
    {
        this->state = st;
    }

    int getId()
    {
        return this->id;
    }

    string getName()
    {
        return this->name;
    }

    string getPwd()
    {
        return this->password;
    }

    string getState()
    {
        return this->state;
    }

private:
    int id;
    string name;
    string password;
    string state;
};

#endif