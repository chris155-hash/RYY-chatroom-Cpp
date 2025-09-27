#ifndef SINGLETON_H
#define SINGLETON_H
#include "global.h"

//单例类模版
template <typename T>
class Singleton{
protected:
    Singleton() = default;    //单例类默认构造不设为public。这里设置protected是想让子类可以调用
    Singleton(const Singleton<T>& st) = delete;
    Singleton& operator = (const Singleton<T>& st) = delete;//拷贝构造和拷贝赋值运算符不允许
    static std::shared_ptr<T> _istance;  //这个单例类我们不想每次都去手动释放，所以用共享指针自动析构。加static扩展至全局
public:
    static std::shared_ptr<T> GetInstance(){
        static std::once_flag s_flag; //保证线程安全。确保只实例化一次，局部变量全局化，只有第一次调用时进行一次初始化
        std::call_once(s_flag,[&] (){
            //这里不用make_shared是因为它需要调用对象的构造函数，这里是单例，构造函数无法访问
            _istance = std::shared_ptr<T> (new T);
        });
        return _istance;
    }

    void PrintAddress(){
        std::cout << _istance.get() << std::endl;
    }

    //CRTP，这里虚函数不用设为virtual。静态多态，减少虚函数开销。避免运行时类型转换
    ~Singleton(){
        std::cout << "this is singleton destruct" << std::endl;
    }
};


template <typename T>
std::shared_ptr<T> Singleton<T>::_istance = nullptr;  //类内声明，类外初始化，这里不能在加static。 初始化静态成员变量。  类型  作用域：：对象
#endif // SINGLETON_H
