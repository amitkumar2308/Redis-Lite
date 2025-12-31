#include "RedisLite.h"
#include <iostream>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

RedisLite::RedisLite(){
     stop = false;
     worker = std::thread(&RedisLite::workerLoop,this);
}

RedisLite::~RedisLite(){
     {
          std::lock_guard<std::mutex> lock(queueMutex);
          stop = true;
     }
     cv.notify_one();
     worker.join();
}

void RedisLite::workerLoop(){
     while(true){
          Command cmd;
          {
               std::unique_lock<std::mutex> lock(queueMutex);
               cv.wait(lock,[&](){
                return !commandQueue.empty() || stop;
               });

               if (stop) {
    while (!commandQueue.empty()) {
        Command &c = commandQueue.front();
        if (c.type == CommandType::GET) {
            c.result.set_value("");
        }
        commandQueue.pop();
    }
    break;
}


          }

          if(cmd.type == CommandType::SET){
               store[cmd.key] = cmd.value;

          }

          else if(cmd.type == CommandType::GET){
               if(store.find(cmd.key)!=store.end())
                  cmd.result.set_value(store[cmd.key]);
               else
                 cmd.result.set_value("");
          }
          else if(cmd.type == CommandType::DEL){
               store.erase(cmd.key);

          }
     }
}



void RedisLite::set(const std::string &key, const std::string &value){
     Command cmd;
     cmd.type = CommandType::SET;
     cmd.key = key;
     cmd.value = value;
     {
     std::lock_guard<std::mutex> lock(queueMutex);
     commandQueue.push(std::move(cmd));
     }
     cv.notify_one();
}

std:: string RedisLite::get(const std::string &key){
     Command cmd;
     cmd.type = CommandType::GET;
     cmd.key = key;

     std::future<std::string> fut = cmd.result.get_future();
     {
     std:: lock_guard<std::mutex> lock(queueMutex);
     commandQueue.push(std::move(cmd));
     }
     cv.notify_one();

     return fut.get();
}

void RedisLite::del(const std::string &key){
    Command cmd;
    cmd.type = CommandType::DEL;
    cmd.key = key;
    {
    std::lock_guard<std::mutex> lock(queueMutex);
     commandQueue.push(std::move(cmd));
    }
    cv.notify_one();
}