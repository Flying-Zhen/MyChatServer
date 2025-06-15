#include "chatservice.hpp"
// #include "user.hpp"
#include "public.hpp"
#include <vector>
#include <iostream>
#include <muduo/base/Logging.h>
using namespace std;
using namespace muduo;
ChatService *ChatService::instance()
{
    static ChatService server;
    return &server;
};
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "do login!!!";
    int id = js["id"];
    string pwd = js["password"];

    User user = _userModel.query(id);

    if (user.getPwd() == pwd && user.getId() == id)
    {
        if (user.getState() == "online")
        {
            json response;
            response["msgid"] = LOG_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "账号已登录";
            conn->send(response.dump());
        }
        else
        {
            {
                lock_guard<mutex> lock(_userConn);
                _userConnMap.insert({id, conn});
            }

            _redis.subscribe(id);

            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOG_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // offlineMsg poll
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                _offlineMsgModel.remove(id);
            }

            // friend push
            vector<User> vu = _frienfModel.query(id);
            if (!vu.empty())
            {
                vector<string> vec1;
                for (User &u : vu)
                {
                    json js;
                    js["id"] = u.getId();
                    js["name"] = u.getName();
                    js["state"] = u.getState();
                    // cout << js << endl;
                    vec1.push_back(js.dump());
                }
                response["friend"] = vec1;
            }

            // group push
            vector<Group> vp = _groupModel.queryGrooups(id);
            if (!vp.empty())
            {
                vector<string> str;
                vector<string> vec1;
                for (Group &g : vp)
                {
                    json js;
                    js["id"] = g.getId();
                    js["name"] = g.getName();
                    js["groupdesc"] = g.getDesc();

                    vector<GroupUser> vuser = g.getUsers();
                    vector<string> vec2;
                    for (GroupUser &gg : vuser)
                    {
                        json js1;
                        js1["id"] = gg.getId();
                        js1["name"] = gg.getName();
                        js1["state"] = gg.getState();
                        js1["role"] = gg.getGroupRole();
                        vec2.push_back(js.dump());
                    }
                    js["users"] = vec2;
                    vec1.push_back(js.dump());
                }
                response["groups"] = vec1;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        json response;
        response["msgid"] = LOG_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户或者密码错误";
        conn->send(response.dump());
    }
};
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "do res!!!";
    string name = js["name"];
    string psw = js["password"];

    User user;
    user.setName(name);
    user.setPwd(psw);

    bool state = _userModel.insert(user);
    if (state)
    {
        json response;
        response["msgid"] = REG_MSG;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG;
        response["errno"] = 1;
        conn->send(response.dump());
    }
};
MsgHandler ChatService::gethandler(int msgid)
{
    auto it = _MsgHandlerMap.find(msgid);
    if (it == _MsgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << " con not find!";
        };
    }
    else
    {
        return this->_MsgHandlerMap[msgid];
    }
};

ChatService::ChatService()
{
    _MsgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _MsgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _MsgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _MsgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _MsgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _MsgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _MsgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    _MsgHandlerMap.insert({LOGINOUT_MEG, std::bind(&ChatService::loginout, this, _1, _2, _3)});

    if (_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handRedisSubscribeMessage, this, _1, _2));
    }
}

void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    User user;
    {
        lock_guard<mutex> lock(_userConn);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                _userConnMap.erase(it);
                user.setId(it->first);
                break;
            }
        }
    }

    _redis.unsubscribe(user.getId());

    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_userConn);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                _userConnMap.erase(it);
                user.setId(it->first);
                break;
            }
        }
    }
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> lock(_userConn);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // online
            cout << js << endl;

            it->second->send(js.dump());
            return;
        }
    }

    // 其他服务器上是否在线
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    // offline
    _offlineMsgModel.insert(toid, js.dump());
}

void ChatService::reset()
{

    _userModel.resetState();
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    _frienfModel.insert(id, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>(); // 这是本身id
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid); // 这个返回id是群里的其他id

    lock_guard<mutex> lock(_userConn);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        User user = _userModel.query(id);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
            return;
        }

        if (user.getState() == "online")
        {
            _redis.publish(id, js.dump());
            return;
        }

        _offlineMsgModel.insert(id, js.dump());
    }
}

void ChatService::handRedisSubscribeMessage(int userid, string msg)
{

    lock_guard<mutex> lock(_userConn);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }
    // 如果在这时候断开链接
    _offlineMsgModel.insert(userid, msg);
}
