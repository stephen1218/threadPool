#include "./include/threadpool.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include "./include/any.h"
#include "./include/semaphore.h"
#include "./include/result.h"
// #if __cplusplus >= 201703L
// #include <any>
// #else
// #include "./include/any.h"
// #endif
// #if __cplusplus >= 202002L
// #include <semaphore>
// #else
// #include "./include/semaphore.h"
// #endif
class myTask:public Task {
private:
    //int data_;
public:
    Any run() {
        std::string str = "task run, thread id:" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        std::cout << str << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return Any((int)1218);
    }
};
//int myTask::taskCount = 0;
int main() {
    Any a1 = (int)123;
    int b1 = a1.cast<int>();
    std::cout << b1 << std::endl;
    ThreadPool* pool = new ThreadPool();
    pool->setMode(PoolMode::MODE_CACHED);   // 设置线程池的工作模式
    pool->setTaskQueMaxThreshHold(10);  // 设置任务队列的大小
    pool->setmaxThreadSize(5);
    pool->start(2); //设置初始线程数量
    std::shared_ptr<Result> res = pool->submitTask(std::make_shared<myTask>());
    for(int i = 0; i < 10; ++i) {
        std::cout << "submit task" << std::endl;
        res = pool->submitTask(std::make_shared<myTask>());
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << res->get().cast<int>() << std::endl;
    delete pool;
    return 0;
}