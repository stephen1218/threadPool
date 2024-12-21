#ifndef ANY_H
#define ANY_H

#include <memory>
class Any
{
public:
    Any() = default;
    template<typename T>
    Any(T value):value_(std::make_unique<Derived<T>>(value)) {}
    Any(Any&) = delete;
    Any& operator=(Any&) = delete;
    Any& operator=(Any&&) = default;
    Any(Any&&) = default;
    template<typename T>
    T cast() {
        Derived<T>* derived = dynamic_cast<Derived<T>*>(value_.get());
        if(derived == nullptr) {
            throw std::runtime_error("type not match");
        }
        return derived->data_;
    }
    class Base {
        public:
            Base() = default;
            virtual ~Base() = default;
    };
    template<typename T>
    class Derived: public Base {
        public:
            T data_;
            Derived(T data):data_(data) {}
    };
private:
    // 存放一个基类指针，可以指向一个派生类对象
    std::unique_ptr<Base> value_;
};

#endif