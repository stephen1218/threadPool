#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <cstddef>
#include <condition_variable>
#include <functional>
#include "result.h"
#include <unordered_map>

// 任务抽象基类
class Task
{
public:
    Task();
    Any res_;
    void setResult(std::shared_ptr<Result> sp);
    // 任务执行接口
    virtual Any run() = 0;
    void exec();
private:
    std::weak_ptr<Result> result_; // 防止交叉引用导致的资源无法释放，因为Result中也包含一个task
};

// 线程池模式
enum class PoolMode
{
    MODE_FIXED,//固定线程数量
    MODE_CACHED,//线程数量可动态增长
};
// PoolMode::MODE_FIXED

// 线程类型
class Thread
{
public:
    Thread(std::function<void(int)>);
    ~Thread();
    // 线程应该不断从任务队列中获取任务并执行
    void start();
    int getId()const;
private:
    std::function<void(int)> func_;
    static int generateId_;
    int threadId_;
};

// 线程池类型
class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    // 设置线程池工作模式
    void setMode(PoolMode mode);

    // 开启线程池
    void start(int initThreadSize);

    // 设置任务队列上限阈值
    void setTaskQueMaxThreshHold(int threshHold);

    // 添加任务
    std::shared_ptr<Result> submitTask(std::shared_ptr<Task> sp);
    
    // 从任务队列中取出任务 , （合并到threadFunc中了）
    // std::shared_ptr<Task> takeTask();
    
    // 设置最大线程数量(cached模式下)
    void setmaxThreadSize(int size);
    // 线程中运行的函数
    void threadFunc(int threadId);

    // 判断线程启动状态
    bool checkRunningState() const;
    // 禁止拷贝构造和赋值
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
private:
    // std::vector<std::unique_ptr<Thread>> threads_;
    // 使用哈希表存放，方便根据线程id来回收线程
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;
    std::mutex threadsMutex_; // 线程池锁
    size_t initThreadSize_; // 初始线程数量
    int maxThreadSize_; // 最大线程数量
    std::atomic<int> idleThreadSize_; // 记录空闲线程数量
    std::atomic_int curThreadSize_; // 当前线程数量
    std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列
    std::atomic<int> taskSize_; // 任务数量
    int taskQueMaxThreshHold_; // 任务队列最大阈值

    std::mutex taskQueMutex_; // 任务队列锁
    std::condition_variable notFull_;// 任务队列非满条件变量
    std::condition_variable notEmpty_;// 任务队列非空条件变量

    PoolMode mode_; // 线程池模式

    std::atomic<bool> isPoolRunning_; // 线程池的启动状态
    
};

#endif // THREADPOOL_H