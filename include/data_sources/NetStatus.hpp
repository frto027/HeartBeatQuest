#pragma once

#include <ctime>
#include <mutex>
#include <string>
struct NetworkStatus {
    std::mutex mutex;

    time_t createTime;
    std::string message;
    NetworkStatus(std::string msg)
        : message(msg) {
        createTime = time(NULL);
    }

    NetworkStatus& operator=(const std::string str) {
        std::lock_guard<std::mutex> g(mutex);
        message = str;
        createTime = time(NULL);
        return *this;
    }

    std::string str() {
        std::lock_guard<std::mutex> g(mutex);
        return message;
    }
};
