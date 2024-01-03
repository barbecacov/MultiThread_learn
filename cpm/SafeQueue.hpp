//
// Created by zd on 1/3/24.
//

#ifndef MY_CC_SAFEQUEUE_HPP
#define MY_CC_SAFEQUEUE_HPP

#include <mutex>
#include <queue>

template<typename T>

class SafeQueue{

public:
    SafeQueue() = default;
    ~SafeQueue() = default;

    SafeQueue(const SafeQueue& other) = delete;

    SafeQueue& operator=(const SafeQueue& other) = delete;

    SafeQueue(SafeQueue&& other) = delete;

    SafeQueue& operator = (const SafeQueue&& other) = delete;

    SafeQueue(const SafeQueue&& other) = delete;

    bool empty(){
        std::unique_lock<std::mutex> locker(m_Mutex);
        return m_Queue.empty();
    }

    int size(){
        std::unique_lock<std::mutex> locker(m_Mutex);
        return m_Queue.size();
    }

    void push(T& value)
    {
        std::unique_lock<std::mutex> locker(m_Mutex);
        m_Queue.emplace(value);
    }

    void push(T&& value)
    {
        std::unique_lock<std::mutex> locker(m_Mutex);
        m_Queue.emplace(std::move(value));
    }


    bool pop(T& value)
    {
        std::unique_lock<std::mutex> locker(m_Mutex);
        if (m_Queue.empty()){
            return false;
        }
        else {
            value = std::move(m_Queue.front());
            m_Queue.pop();
            return true;
        }
    }

private:
    std::mutex m_Mutex;
    std::queue<T> m_Queue;

};

#endif //MY_CC_SAFEQUEUE_HPP
