#include "./include/threadpool.h"
#include <iostream>
#include <thread>
#include "./include/result.h"

void Result::init() {
        task_->setResult(shared_from_this());
    }
Task::Task(): result_() {}

void Task::exec() {
    if(result_.use_count() != 0) {
        std::shared_ptr<Result> sp = result_.lock();
        sp->setVal(run());
    }
}

void Task::setResult(std::shared_ptr<Result> sp) {
    result_ = sp;
}

Thread::Thread(std::function<void(int)> func):func_(func),threadId_(generateId_++) {}

int Thread::generateId_ = 0; // 静态成员函数在类外初始化

int Thread::getId()const {
    return threadId_;
}
Thread::~Thread(){}
void Thread::start() {
    std::thread t(func_,threadId_);
    t.detach();
}
ThreadPool::ThreadPool():initThreadSize_(2)
                        ,taskSize_(0)
                        ,taskQueMaxThreshHold_(1024)
                        ,mode_(PoolMode::MODE_FIXED)
                        ,isPoolRunning_(false)
                        ,idleThreadSize_(0)
{
}
ThreadPool::~ThreadPool() {

}

// 设置线程池工作模式
void ThreadPool::setMode(PoolMode mode) {
    if(checkRunningState()) return;
    mode_ = mode;
}

// 开启线程池
void ThreadPool::start(int initThreadSize){
    isPoolRunning_ = true;
    // 记录初始线程个数
    initThreadSize_ = initThreadSize;
    curThreadSize_ = initThreadSize;
    // 创建初始线程
    for(int i = 0; i < initThreadSize_; i++) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        threads_.insert({ptr->getId(),std::move(ptr)});
        idleThreadSize_++;
    }

    // 启动线程
    for(const auto& [id, td] : threads_) {
        td->start();
    }
}

void ThreadPool::threadFunc(int threadId)
{
    // 记录线程开始进入空闲状态的时刻
    auto lastTime = std::chrono::high_resolution_clock().now();
    while (true)
    {
        std::unique_lock<std::mutex> lk(taskQueMutex_);
        if (mode_ == PoolMode::MODE_CACHED)
        {
            
            while (taskQue_.size() == 0) // todo: 确认一下这里是等于0还是大于0
            {
                // 条件变量，超时返回
                if (std::cv_status::timeout ==
                    notEmpty_.wait_for(lk, std::chrono::seconds(1)))
                {
                    auto now = std::chrono::high_resolution_clock().now();
                    auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                    if (dur.count() >= 60 && curThreadSize_ > initThreadSize_) {
                        // 超过最大空闲时间，开始回收线程
                        std::cout<< "回收线程" << curThreadSize_ << std::endl;
                        // 记录线程数量的相关变量的值修改
                        curThreadSize_--;
                        // 从线程池容器threads_中删除
                        threads_.erase(threadId);
                    }
                }
            }
        }
        else
        {
            notEmpty_.wait(lk, [this]()
                           { return taskSize_ > 0; });
        }
        idleThreadSize_--; // 取到任务时需要减少空闲线程的数量
        auto sp = taskQue_.front();
        taskQue_.pop();
        taskSize_--;
        lk.unlock();

        notFull_.notify_all();

        sp->exec();
        lastTime = std::chrono::high_resolution_clock().now(); // 更新开始空闲的时间
        idleThreadSize_++;                                     // 执行完任务，增加空闲线程的数量
    }
    // std::cout << "thread end!" << std::endl;
}
// 设置任务队列上限阈值
void ThreadPool::setTaskQueMaxThreshHold(int threshHold){
    if(checkRunningState()) return;
    taskQueMaxThreshHold_ = threshHold;
}

// todo
// 添加任务
std::shared_ptr<Result> ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    std::unique_lock<std::mutex> lk(taskQueMutex_);
    // 防止阻塞用户线程
    if(taskSize_ >= taskQueMaxThreshHold_) {
        std::cv_status status = notFull_.wait_for(lk, std::chrono::seconds(1));
        if(status == std::cv_status::timeout) {
            std::cerr << "task queue is full, submit task fail" << std::endl;
            // todo：定义返回类型
            return std::make_shared<Result>(sp, false);
        }
    }
    taskQue_.push(sp);
    taskSize_++;
    lk.unlock();
    notEmpty_.notify_one();
    //todo 定义返回类型
    auto res = std::make_shared<Result>(sp, true);
    res->init();
    // 判断是否需要创建线程
    if (mode_ == PoolMode::MODE_CACHED
        && taskSize_ > idleThreadSize_
        && curThreadSize_ < maxThreadSize_) {
        
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this,std::placeholders::_1));
        threads_.insert({ptr->getId(),std::move(ptr)});
        std::cout << "创建线程" << ptr->getId() << std::endl;
        ptr->start();
        idleThreadSize_++;
        curThreadSize_++;
    }
    return std::move(res);
}



bool ThreadPool::checkRunningState() const {
    return isPoolRunning_;
}

void ThreadPool::setmaxThreadSize(int size) {
    if(checkRunningState() || mode_ == PoolMode::MODE_CACHED) return ;
    maxThreadSize_ = size;
}
