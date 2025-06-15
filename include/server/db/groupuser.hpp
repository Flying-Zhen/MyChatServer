#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

class GroupUser : public User
{
public:
    void setRole(string gr)
    {
        this->role = gr;
    }
    string getGroupRole()
    {
        return this->role;
    }

private:
    string role;
};

#endif