#include <iostream>
#include "redis.hpp"
using namespace std;

Redis::Redis() : _pulish_context(nullptr), _subcribe_context(nullptr)
{
}

Redis::~Redis()
{
    if (_pulish_context != nullptr)
    {
        redisFree(_pulish_context);
    }
    if (_subcribe_context != nullptr)
    {
        redisFree(_subcribe_context);
    }
}

bool Redis::connect()
{
    _pulish_context = redisConnect("127.0.0.1", 6379);
    if (_pulish_context == nullptr)
    {
        cerr << "redis connect fail" << endl;
        return false;
    }

    _subcribe_context = redisConnect("127.0.0.1", 6379);
    if (_subcribe_context == nullptr)
    {
        cerr << "redis connect fail" << endl;
        return false;
    }

    thread t([&]
             { observer_channel_message(); });
    t.detach();

    cout << "redis connect success" << endl;
    return true;
}

bool Redis::publish(int channel, string msg)
{
    redisReply *reply = (redisReply *)redisCommand(_pulish_context, "PUBLISH %d %s", channel, msg.c_str());
    if (reply == nullptr)
    {
        cerr << "publish fail" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}
bool Redis::subscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe fail" << endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "subscribe fail" << endl;
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe fail" << endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "unsubscribe fail" << endl;
            return false;
        }
    }
    return true;
}
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void **)&reply))
    {
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    cerr << ">>>>>>>>>>>observer_channel_message quit<<<<<<<<<<<<<<<<<" << endl;
}

void Redis::init_notify_handler(function<void(int, string)> fn)
{
    this->_notify_message_handler = fn;
}
