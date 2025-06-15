#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "user.hpp"
#include "group.hpp"
#include "public.hpp"

// 记录打当前用户信息
User g_currentUser;
// 记录当前用户好友列表
vector<User> g_currentFriendList;
// 记录用户群组列表
vector<Group> g_currentGroupList;
// 显示用户基本信息
void showCurrentUserData();
// 控制主菜单运行
bool isMainMemuRunning = false;
// 接受线程
void readTaskHandler(int clientfd);
// 获取系统时间
string getCurrentTime();
// 主聊天界面
void mainMenu(int clientfd);

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid!" << endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建client的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    // 填写server的ip+port
    sockaddr_in server;
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // 链接server
    if (connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)) == -1)
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    for (;;)
    {
        cout << "===================" << endl;
        cout << "1.login" << endl;
        cout << "2.register" << endl;
        cout << "3.quit" << endl;
        cout << "===================" << endl;
        cout << "chioce:";
        int chioce = 0;
        cin >> chioce;
        cin.get();

        switch (chioce)
        {
        case 1:
        {
            int id = 0;
            char pwd[50];
            cout << "userid";
            cin >> id;
            cin.get();
            cout << "userpwd";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send login msg erroe" << endl;
            }
            else
            {
                // cout << "2" << endl;
                char buffer[1024] = {0}; // 不初始化后面json解析会出错
                len = recv(clientfd, buffer, 1024, 0);
                if (len == -1)
                {
                    cerr << "login msg error" << endl;
                }
                else
                {
                    // cout << "3" << endl;
                    json response = json::parse(buffer);
                    // cout << response << endl;
                    if (response["errno"].get<int>() != 0)
                    {
                        cerr << response["errmsg"] << endl;
                    }
                    else
                    {
                        // cout << "5" << endl;
                        g_currentUser.setId(response["id"].get<int>());
                        g_currentUser.setName(response["name"]);
                        // cout << "1" << endl;
                        // 如果有好友
                        if (response.contains("friend"))
                        {
                            g_currentFriendList.clear();
                            vector<string> vec = response["friend"];
                            for (string &str : vec)
                            {
                                json js = json::parse(str);
                                User user;
                                // cout << 1 << endl;
                                user.setId(js["id"].get<int>());
                                // cout << 2 << endl;
                                user.setName(js["name"]);
                                cout << 3 << endl;
                                cout << js["state"] << endl;
                                user.setState(js["state"]);
                                cout << 4 << endl;
                                g_currentFriendList.push_back(user);
                                cout << 5 << endl;
                            }
                        }
                        // 如果有群组
                        if (response.contains("groups"))
                        {
                            g_currentGroupList.clear();
                            vector<string> vec1 = response["groups"];
                            for (string &groupstr : vec1)
                            {
                                json grpjs = json::parse(groupstr);
                                Group group;
                                group.setId(grpjs["id"].get<int>());
                                group.setName(grpjs["name"]);
                                group.setDesc(grpjs["groupdesc"]);

                                vector<string> vec2 = grpjs["users"];
                                for (string &userstr : vec2)
                                {
                                    GroupUser user;
                                    json js = json::parse(userstr);
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    user.setRole(js["role"]);
                                    group.getUsers().push_back(user);
                                }

                                g_currentGroupList.push_back(group);
                            }
                        }

                        showCurrentUserData();

                        // 有离线消息
                        if (response.contains("offlinemsg"))
                        {
                            vector<string> vec = response["offlinemsg"];
                            for (string str : vec)
                            {
                                json js = json::parse(str);
                                if (js["msgid"].get<int>() == ONE_CHAT_MSG)
                                {
                                    cout << js["time"].get<string>() << " [" << js["id"] << "] " << js["name"].get<string>()
                                         << " said: " << js["message"].get<string>() << endl;
                                }
                                else
                                {
                                    cout << "群消息： [" << js["id"] << "]" << js["time"].get<string>() << js["name"].get<string>()
                                         << " said: " << js["message"].get<string>() << endl;
                                }
                            }
                        }

                        static int readthreadnum = 0;
                        if (readthreadnum == 0)
                        {
                            std::thread readTask(readTaskHandler, clientfd);
                            readTask.detach();
                        }
                        isMainMemuRunning = true;
                        mainMenu(clientfd);
                    }
                }
            }
            break;
        };
        case 2:
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin.getline(name, 50);
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg err" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if (len == -1)
                    cerr << "recv reg msg error" << endl;
                else
                {
                    json response = json::parse(buffer);
                    if (response["errno"].get<int>() != 0)
                    {
                        cerr << name << " is already exist" << endl;
                    }
                    else
                    {
                        cout << name << " register success,userid is " << response["id"] << endl;
                    }
                }
            }
            break;
        }
        case 3:
        {
            close(clientfd);
            exit(0);
        }
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }
}

void readTaskHandler(int client)
{
    for (;;)
    {
        char buffer[1024];
        int len = recv(client, buffer, 1024, 0);
        if (len == 0 || len == -1)
        {
            close(client);
            exit(-1);
        }
        json js = json::parse(buffer);
        if (js["msgid"].get<int>() == ONE_CHAT_MSG)
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "] " << js["name"].get<string>()
                 << " said: " << js["message"].get<string>() << endl;
            continue;
        }
        if (js["msgid"].get<int>() == GROUP_CHAT_MSG)
        {
            cout << "群消息： [" << js["groupid"] << "]" << js["time"].get<string>() << js["name"].get<string>()
                 << " said: " << js["message"].get<string>() << endl;
            continue;
        }
    }
}

void help(int clientfd = 0, string = "");

void chat(int clientfd, string msg);

void addfriend(int clientfd, string msg);
void creategroup(int clientfd, string msg);
void addgroup(int clientfd, string msg);
void groupchat(int clientfd, string msg);
void loginout(int clientfd = 0, string msg = "");
unordered_map<string, string> commandMap = {
    {"help", "所有命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式quit"}};

unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

void mainMenu(int clientfd)
{
    help();

    char buffer[1024];
    while (isMainMemuRunning)
    {
        cin.getline(buffer, 1024);
        string combf(buffer);
        string cmd;
        int idx = combf.find(":");
        if (idx == -1)
        {
            cmd = combf;
        }
        else
        {
            cmd = combf.substr(0, idx);
        }

        auto it = commandHandlerMap.find(cmd);
        if (it == commandHandlerMap.end())
        {
            cout << "invalid commend" << endl;
            continue;
        }
        it->second(clientfd, combf.substr(idx + 1, combf.size() - idx));
    }
}

void showCurrentUserData()
{
    cout << "=================login user====================" << endl;
    cout << "current login user id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "------------------friend list-------------------" << endl;
    if (!g_currentFriendList.empty())
    {
        for (User &user : g_currentFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "------------------group list-------------------" << endl;
    if (!g_currentGroupList.empty())
    {
        for (Group &group : g_currentGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;

            for (GroupUser user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << user.getState() << " " << user.getGroupRole() << endl;
            }
        }
    }
    cout << "================================================" << endl;
}

string getCurrentTime()
{
    time_t now = time(nullptr);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buf);
}

void help(int clientfd, string)
{
    cout << "command list" << endl;
    for (auto it = commandMap.begin(); it != commandMap.end(); it++)
    {
        cout << it->first << ":" << it->second << endl;
    }
}

void chat(int clientfd, string msg)
{
    int idx = msg.find(":");
    if (idx == -1)
    {
        cerr << "chat command invalid" << endl;
    }

    int friendid = atoi(msg.substr(0, idx).c_str());
    string _msg = msg.substr(idx + 1, msg.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["to"] = friendid;
    js["message"] = _msg;
    js["time"] = getCurrentTime();
    string bf = js.dump();

    int len = send(clientfd, bf.c_str(), strlen(bf.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send chat msg error" << bf << endl;
    }
}

void addfriend(int clientfd, string msg)
{
    int friendid = atoi(msg.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string str = js.dump();

    int len = send(clientfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send add msg error" << str << endl;
    }
}
void creategroup(int clientfd, string msg)
{
    int idx = msg.find(":");

    if (idx == -1)
    {
        cerr << "creategroup command invalid" << endl;
    }

    string groupname = msg.substr(0, idx);
    string groupdesc = msg.substr(idx + 1, msg.size() - idx);

    json js;
    js["id"] = g_currentUser.getId();
    js["msgid"] = CREATE_GROUP_MSG;
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string str = js.dump();

    int len = send(clientfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send creategroup msg error" << str << endl;
    }
}
void addgroup(int clientfd, string msg)
{
    int groupid = atoi(msg.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string str = js.dump();

    int len = send(clientfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addgroup msg error" << str << endl;
    }
}
void groupchat(int clientfd, string msg)
{
    int idx = msg.find(":");
    if (idx == -1)
    {
        cerr << "groupchat command invalid" << endl;
    }

    int groupid = atoi(msg.substr(0, idx).c_str());
    string _msg = msg.substr(idx + 1, msg.size() - idx);

    json js;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    js["msgid"] = GROUP_CHAT_MSG;
    js["time"] = getCurrentTime();
    js["name"] = g_currentUser.getName();
    js["message"] = _msg;
    string str = js.dump();

    int len = send(clientfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send groupchat msg error" << str << endl;
    }
}
void loginout(int clientfd, string msg)
{
    json js;
    js["id"] = g_currentUser.getId();
    js["msgid"] = LOGINOUT_MEG;
    string str = js.dump();

    int len = send(clientfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send loginout msg error" << str << endl;
    }
    else
    {
        isMainMemuRunning = false;
    }
}
