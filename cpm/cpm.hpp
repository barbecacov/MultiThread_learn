//
// Created by zd on 12/21/23.
//

#ifndef MY_CC_CPM_HPP
#define MY_CC_CPM_HPP

// Consumer Producer

#include <algorithm>
#include <condition_variable>
#include <future>
#include <memory>
#include <queue>
#include <thread>

using namespace std;

// 1. queue is full, producer stop produce product
// 2. queue is empty, consumer stop buy product
// 3. queue is not full and not empty, all is running

namespace cpm{

    template<typename Result, typename Input>

    class Instance {

    protected:
        struct Item {
            Input input;
            shared_ptr<promise<Result>> pro;
        };

        condition_variable empty_cond_;
        condition_variable full_cond_;
        queue<Item> input_queue_;
        mutex queue_lock_;
        shared_ptr<thread> worker_;

        atomic<bool> run_{false};

        // the max size of queue
        volatile int max_items_cap_ = 0;

    public:
        virtual ~Instance() {stop();}

        void stop() {

            run_ = false;
            empty_cond_.notify_all();
            full_cond_.notify_all();
            {
                unique_lock<mutex> l(queue_lock_);
                while (!input_queue_.empty())
                {
                    auto &item = input_queue_.front();
                    if (item.pro) item.pro->set_value(Result());
                    input_queue_.pop();
                }
            }

            if (worker_)
            {
                worker_->join();
                worker_.reset();
            }
        }


        virtual shared_future<Result> commit(const Input &input)
        {
            Item item;
            item.input = input;
            item.pro.reset(new promise<Result>());

            {
                unique_lock<mutex> _lock_(queue_lock_);
                full_cond_.wait(_lock_, [&]() { return !run_ || input_queue_.size() < max_items_cap_; });
                input_queue_.push(item);
            }

            empty_cond_.notify_one();
            return item.pro->get_future();
        }

        bool start(int max_items_cap)
        {
            stop();
            this->max_items_cap_ = max_items_cap;
            promise<bool> status;
            worker_ = make_shared<thread>(&Instance::worker, this, std::ref(status));
            return status.get_future().get();
        }

    private:

        void worker(promise<bool> &status)
        {

            run_ = true;
            status.set_value(true);

            Item fetch_item;
            Input input;
            while (get_item_and_wait(fetch_item))
            {

                input = fetch_item.input;

                cout << "h : " << input.height << endl;

                cout << "222" << endl;

                fetch_item.pro->set_value(Result());

            }
            run_ = false;
        }


        virtual bool get_item_and_wait(Item &fetch_item) {

            unique_lock<mutex> l(queue_lock_);
            empty_cond_.wait(l, [&]() {return !run_ || !input_queue_.empty();});

            if (!run_) return false;

            fetch_item = std::move(input_queue_.front());
            input_queue_.pop();
            full_cond_.notify_one();
            return true;
        }
    };
}

#endif //MY_CC_CPM_HPP

