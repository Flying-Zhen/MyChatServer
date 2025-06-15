#include "chatserver.hpp"
#include <string>
#include <functional>
#include <nlohmann/json.hpp>
#include "chatservice.hpp"
#include <iostream>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg)
{
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
    _server.setThreadNum(4);
}
void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buff,
                           Timestamp time)
{
    string buf = buff->retrieveAllAsString();
    json js = json::parse(buf);
    // 需要讲网络与业务解耦，收到网络传来的参数获取相应的handler
    auto handler = ChatService::instance()->gethandler(js["msgid"].get<int>());
    cout << "bug1" << endl;
    handler(conn, js, time);
    cout << "bug" << endl;
    // cout << js.dump() << endl;
}
