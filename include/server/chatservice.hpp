#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <mutex>
#include <functional>
#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <nlohmann/json.hpp>
#include "usermodel.hpp"
#include "offlinemsgmodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

class ChatService
{
public:
    static ChatService *instance();
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    MsgHandler gethandler(int msgid);
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void clientCloseException(const TcpConnectionPtr &conn);
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void reset();
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void handRedisSubscribeMessage(int userid, string msg);

private:
    ChatService();
    std::unordered_map<int, MsgHandler> _MsgHandlerMap;
    UserModel _userModel;
    std::unordered_map<int, TcpConnectionPtr> _userConnMap;
    mutex _userConn;
    offlineMsgModel _offlineMsgModel;
    friendModel _frienfModel;
    groupModel _groupModel;
    Redis _redis;
};

#endif