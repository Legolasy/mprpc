#pragma once
#include<queue>
#include<mutex>
#include<condition_variable>
#include<thread>

template<typename T>
class LockQueue
{
public:
    void Push(const T&t){
        std::lock_guard<std::mutex>lock(m_mutex);
        m_queue.push(t);
        m_condvariable.notify_one();
    }
    T Pop(){
        std::unique_lock<std::mutex>lock(m_mutex);
        while(m_queue.empty())
        {
            m_condvariable.wait(lock); // 让出锁 等待队列里有东西
        }
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }
    
private:
    std::queue<T>m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
};