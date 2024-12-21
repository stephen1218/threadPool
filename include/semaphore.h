#ifndef THREADPOOL_SEMAPHORE_H
#define THREADPOOL_SEMAPHORE_H

#include <mutex>
#include <condition_variable>
//#include <condition_variable>

class Semaphore {
public:
    Semaphore() = default;
    Semaphore(int resLimit):resLimit_(resLimit){}
    // 资源计数减1
    void wait() {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [&](){return resLimit_ > 0;});
        resLimit_--;
    }
    // 资源计数加1
    void post(){
        std::unique_lock<std::mutex> lock(mtx_);
        resLimit_++;
        cond_.notify_all();
    }
private:
    int resLimit_;
    std::mutex mtx_;
    std::condition_variable cond_;
};

#endif //THREADPOOL_SEMAPHORE_H