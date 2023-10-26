//
// Created by zd on 10/26/23.
//

#ifndef MY_CC_BLOCKQUEUE_H
#define MY_CC_BLOCKQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

using namespace std;

template<typename E>
class BlockQueue
{
private:
    std::mutex _mtx;
    std::condition_variable _cond;
    int _max_size;
    std::queue<E> _queue;

public:
    BlockQueue(int max_size): _max_size(max_size)
    {

    }

    void push(E &&e)
    {

        std::unique_lock<std::mutex> lock(_mtx);

        _cond.wait(lock, [this](){ return _queue.size() < _max_size; });

        _queue.push(std::move(e));

        lock.unlock();
        _cond.notify_one();
    }

    E front()
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _cond.wait(lock, [this](){ return !_queue.empty(); });

        return std::move(_queue.front());
    }

    void pop()
    {
        std::unique_lock<std::mutex> lock(_mtx);

        _cond.wait(lock, [this](){ return !_queue.empty(); });

        _queue.pop();

        lock.unlock();
        _cond.notify_one();
    }

    int size()
    {
        std::lock_guard<std::mutex> lock(_mtx);
        return _queue.size();
    }
};


#endif //MY_CC_BLOCKQUEUE_H
