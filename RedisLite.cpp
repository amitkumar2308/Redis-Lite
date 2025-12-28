#include "RedisLite.h"

void RedisLite::set(const std::string &key, const std::string &value){
     std::lock_guard<std::mutex> lock(mtx);
     store[key]  = value;
}

std:: string RedisLite::get(const std::string &key){
     std:: lock_guard<std::mutex> lock(mtx);
     if(store.find(key)!= store.end()){
        return store[key];
     }
     return " "; //not found
}

void RedisLite::del(const std::string &key){
    std::lock_guard<std::mutex> lock(mtx);
     store.erase(key);
}