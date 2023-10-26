#include <thread>
#include "BlockQueue.h"
#include <iostream>


int main()
{
    int size_data = 100;

    BlockQueue<int> qu(3);

    std::thread t1([&](){
        for(int i = 0; i < size_data; i++)
        {
            qu.push(std::move(i));
        }
    });

    std::thread t2([&](){
        for(int i = 0; i < size_data; i++)
        {
            auto item = qu.front();
            qu.pop();
            std::cout << "consumed " << item << std::endl;
        }
    });

    t1.join();
    t2.join();

    return 0;

}