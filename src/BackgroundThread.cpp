
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include "BackgroundThread.hpp"

std::unique_ptr<std::thread> background_thread_ptr = nullptr;
std::condition_variable background_wait;

std::mutex background_task_queue_mutex;
std::queue<std::function<void(void)>> background_task_queue;
std::atomic<bool> background_terminated = false;

namespace HeartBeat{

    void background_thread_main(){
        std::unique_lock<std::mutex> lock(background_task_queue_mutex);
        while(!background_terminated){
            background_wait.wait(lock, [](){ 
                return !background_task_queue.empty() || background_terminated;
            });
            if(background_terminated){
                return;
            }

            while(!background_task_queue.empty()){
                auto func = std::move(background_task_queue.front());
                background_task_queue.pop();
                lock.unlock();
                func(); // make sure the func is lock-free
                lock.lock();
            }
        }
    }

    void runBackground(std::function<void(void)> func){
        static std::once_flag bg_create;

        std::call_once(bg_create, [](){
            background_thread_ptr = std::unique_ptr<std::thread>(new std::thread(background_thread_main));
        });

        {
            std::lock_guard<std::mutex> lock(background_task_queue_mutex);
            background_task_queue.push(std::move(func));
        }
        background_wait.notify_one();
    }

    void terminateBackground(){
        background_terminated = true;
        background_wait.notify_one();
    }
}