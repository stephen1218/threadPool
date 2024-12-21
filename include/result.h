#ifndef THREADPOOL_RESULT
#define THREADPOOL_RESULT

#include "any.h"
#include "semaphore.h"
#include "threadpool.h"
class Task;
class Result : public std::enable_shared_from_this<Result>
{
private:
    Any data_;
    std::shared_ptr<Task> task_;
    Semaphore sem_;
    std::atomic_bool isValid_;
public:
    Result(std::shared_ptr<Task> task, bool isValid = true):
            task_(task), 
            isValid_(isValid),
            sem_(0)
    {
        // shared_from_this 不能在构造函数中使用，否则会发生未定义行为, 因此添加init函数
        // task_->setResult(shared_from_this());
    }
    void init();
    Result() = default;
    Result(Result&&) = default;
    Result& operator=(Result&&) = default;
    void setVal(Any data) {
        data_ = std::move(data);
        sem_.post();
    }
    // 获取返回值
    Any get() {
        if(!isValid_) {
            throw std::runtime_error("not valid");
            return Any();
        }
        // 如果还没有设置返回值，就阻塞
        sem_.wait();
        return std::move(data_);
    }

};

#endif // THREADPOOL_RESULT