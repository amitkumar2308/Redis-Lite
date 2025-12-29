#pragma once
#include<unordered_map>
#include<queue>
#include<string>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<future>
enum class CommandType{
   SET,
   GET,
   DEL
};

struct Command{
   CommandType type;
   std::string key;
   std::string value;

   std::promise<std::string> result;
};

class RedisLite{
    private:
       std::unordered_map<std::string,std::string> store;
       std::queue<Command> commandQueue;
       std::mutex queueMutex;
       std::condition_variable cv;

       std::thread worker;
       bool stop;

       void workerLoop();

    public:
      RedisLite();
      ~RedisLite();
       void set(const std::string &key, const std::string &value);
       std::string get(const std::string &key);
       void del(const std::string &key);
};


