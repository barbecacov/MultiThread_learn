#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>


using namespace std;

class GLTaskDispatch{

public:

    static GLTaskDispatch& getInstance(){
        static GLTaskDispatch t;
        return t;
    }

    bool start(){

        auto func = [this]() {

            while (!_interrupt.load())
            {
                std::function<void()> task;

                {

                    std::unique_lock<mutex> lock(this->_taskMutex);

                    this->_taskCv.wait(lock, [this] { return this->_interrupt.load() || !this->_tasks.empty();});

                    if (this->_interrupt.load())
                    {
                        continue;
                    }

                    task = std::move(this->_tasks.front());
                    this->_tasks.pop();

                }

                task();
            }


        };

        _thread = std::make_unique<std::thread>(func);

        return true;

    }

    bool stop(){
        _interrupt.store(true);
        this->_taskCv.notify_all();

        if (_thread && _thread->joinable())
        {
            _thread->join();
        }

        return true;

    }

    /// @note 可变参模板函数【尾随返回类型】
    /// 将函数（可调用实体）打包为任务放进任务队列，并返回 future 以便查询任务执行结果
    template <typename F, typename... Args>
    auto run(F&& f, Args &&...args)
    -> std::shared_ptr<std::future<std::result_of_t<F(Args...)>>> { ///< 返回值类型
        /// @note 类型重定义
        using returnType = std::result_of_t<F(Args...)>; ///< 推导出可调用对象的返回值类型

        /// @note 将外部传进来的可调用对象及其参数打包
        auto task = std::make_shared<std::packaged_task<returnType()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<returnType> ret = task->get_future();
        {
            std::lock_guard<std::mutex> lock(this->_taskMutex);

            std::cout << "_tasks emplace !" << std::endl;
            this->_tasks.emplace([task]() {
                (*task)();
            }); ///< 将打包的 task 以 lambda 表达式的形式（满足 function<void()> ）进行封装（在内部调用 task ），并将这个封装的可调用实体存进任务队列当中
        }

        this->_taskCv.notify_all(); ///< 通知所有等待条件变量的子线程

        /// @note 返回 future 给函数调用者，以便获取返回值
        return std::make_shared<std::future<std::result_of_t<F(Args...)>>>(
                std::move(ret));
    }


private:

    GLTaskDispatch() {};
    std::unique_ptr<thread> _thread = nullptr;
    std::atomic<bool> _interrupt{ false };
    std::queue<std::function<void()>> _tasks;
    std::mutex _taskMutex;
    std::condition_variable _taskCv;
};

void func1() {
    for (int i = 0; i < 20; i++) {
        std::cout << "func1 " << i << "\n";
    }
}

int func2() {
    for (int i = 0; i < 20; i++) {
        std::cout << "func2 " << i << "\n";
    }
    return 64;
}


int main() {
    GLTaskDispatch& t = GLTaskDispatch::getInstance();
    t.start(); ///< 启动唯一的 GL 子线程

    ///  @note GL 子线程添加了 func1 任务，并 get 等待其执行完毕
    t.run(func1)->get();
    std::cout << "func1 return" << std::endl;

    ///  @note GL 子线程添加了 func2 任务，并 get 等待其执行完毕
    int d = t.run(func2)->get();
    std::cout << "func2 return " << d << std::endl;

    /// @note 主线程干点别的任务
    std::this_thread::sleep_for(std::chrono::seconds(2));

    t.stop(); ///< GL 子线程终止

    return 0;
}