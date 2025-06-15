#include "chatserver.hpp"
#include <iostream>
#include <signal.h>
#include "chatservice.hpp"

using namespace std;

void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress inet(argv[1], atoi(argv[2]));
    ChatServer server(&loop, inet, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}