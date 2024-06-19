#include <iostream>

using namespace std;

/*
 这是一个使用模板实现的单例模式。单例模式是一种设计模式，用于确保一个类只有一个实例，并提供一个全局访问点。在这个实现中，我们使用了模板来支持不同类型的单例对象。
getInstance 方法是静态方法，用于获取单例对象的引用。由于静态方法在编译时被绑定到类，而不是实例，因此这个方法可以被外部直接调用，而不需要创建类的实例。
为了防止创建多个单例对象，我们使用了静态变量 instance，它会自动初始化，且只会被创建一次。当我们调用 getInstance 方法时，它会返回这个静态变量的引用。
我们还删除了拷贝构造函数和赋值运算符，以防止通过这些方法创建新的单例对象。
Singleton 类的构造函数和析构函数是受保护的，这意味着它们只能被继承的子类调用。这是为了确保单例对象在全局范围内被创建和销毁，而不是在局部范围内。
注意，这个实现假设 T 类型具有默认构造函数。如果 T 类型没有默认构造函数，那么需要为 Singleton 类提供相应的构造函数。
 */

template<typename T>
class Singleton{

public:
    // 获取单例对象的静态方法
    static T& getInstance(){
        // 静态变量，只会被创建一次
        static T instance;
        // 返回单例对象
        return instance;
    }

    // 删除拷贝构造函数
    Singleton(const Singleton&) = delete;
    // 删除赋值运算符
    Singleton& operator=(const Singleton&) = delete;

protected:
    // 构造函数
    Singleton() = default;
    // 析构函数
    ~Singleton() = default;

};

class Test:public Singleton<Test>
{
public:
    void myprint()
    {
        std::cout<<"test Singleton"<<std::endl;
    }
};


class TestA
{
public:
    void myprint()
    {
        std::cout<<"test Singleton"<<std::endl;
    }
};

int main(){

    Singleton<Test>::getInstance().myprint();
    Test::getInstance().myprint();
    return 0;

}
