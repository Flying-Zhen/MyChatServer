#ifndef REDIS_HPP
#define REDIS_HPP
#include <thread>
#include <hiredis/hiredis.h>
#include <functional>
#include <string>
using namespace std;
class Redis
{
public:
    Redis();
    ~Redis();

    bool connect();
    bool publish(int channel, string msg);
    bool subscribe(int channel);
    bool unsubscribe(int channel);
    // 在独立线程中接受订阅通道的消息
    void observer_channel_message();

    // 初始化向业务层上报消息的回调对象
    void init_notify_handler(function<void(int, string)> fn);

private:
    redisContext *_pulish_context;

    redisContext *_subcribe_context;

    function<void(int, string)> _notify_message_handler;
};

#endif