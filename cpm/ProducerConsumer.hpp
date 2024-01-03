//
// Created by zd on 1/3/24.
//

#ifndef MY_CC_PRODUCERCONSUMER_HPP
#define MY_CC_PRODUCERCONSUMER_HPP

#include "SafeQueue.hpp"
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>

template<typename T>
class ProducerConsumer{

public:

    ProducerConsumer() :m_QueueMaxSize(20), m_ProducerThreads(2), m_ConsumerThreads(2), m_RunningStatus(true)
    {
        initialize();
    }

    ProducerConsumer(int queueMaxSize, int producerNum, int consumerNum) :m_QueueMaxSize(queueMaxSize),
    m_ProducerThreads(producerNum), m_ConsumerThreads(consumerNum), m_RunningStatus(true){
        initialize();
    }

    ~ProducerConsumer()
    {
        m_RunningStatus = false;

        m_QueueNotEmptyCV.notify_all();
        m_QueueNotFullCV.notify_all();

        for (auto& thread: m_ProducerThreads) {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        for (auto& thread: m_ConsumerThreads) {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }


private:
    SafeQueue<T> m_Queue;

    int m_QueueMaxSize;

    std::condition_variable m_QueueNotFullCV;
    std::condition_variable m_QueueNotEmptyCV;

    std::vector<std::thread> m_ProducerThreads;
    std::vector<std::thread> m_ConsumerThreads;

    std::mutex m_Mutex;

    std::atomic<bool> m_RunningStatus{};

    void initialize(){
        for (auto & m_ProducerThread : m_ProducerThreads) {

            m_ProducerThread = std::thread(&ProducerConsumer::producer, this);

        }

        for (auto & m_ConsumerThread: m_ConsumerThreads) {
            m_ConsumerThread = std::thread(&ProducerConsumer::consumer, this);
        }
    }

    bool isFull(){
        if (m_Queue.size() >= m_QueueMaxSize){
            return true;
        }
        return false;
    }

    void producer(){

        while (m_RunningStatus)
        {
            std::unique_lock<std::mutex> locker(m_Mutex);
            if (isFull()){
                std::cout << "Queue is full ! Waiting for m_QueueNotFullCV" << std::endl;
                m_QueueNotFullCV.wait(locker);
            }

            if (!isFull())
            {
                T val = 3;
                m_Queue.push(val);
                m_QueueNotEmptyCV.notify_one();
            }
        }

    }

    void consumer(){

        while (m_RunningStatus)
        {
            std::unique_lock<std::mutex> locker(m_Mutex);
            if (m_Queue.empty())
            {
                std::cout << "Queue is empty ! Waiting for m_QueueNotEmptyCV" << std::endl;
                m_QueueNotEmptyCV.wait(locker);
            }

            if (!m_Queue.empty())
            {
                T val;
                bool result = m_Queue.pop(val);
                val++;
                std::cout << "result : " << val << std::endl;
                m_QueueNotFullCV.notify_one();
            }
        }

    }

};


#endif //MY_CC_PRODUCERCONSUMER_HPP
