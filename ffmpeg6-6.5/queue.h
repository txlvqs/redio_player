#ifndef QUEUE_H
#define QUEUE_H

#include <mutex>//互斥量头文件
#include <condition_variable>//条件变量，实现线程同步，满足条件唤醒
#include <queue>//c++队列容器

using namespace std;

template <typename T>//模板类声明
class Queue
{
public:
//    Queue();
//    ~Queue();

    void Abort()
    {
        abort_ = 1;
        cond_.notify_all();//notify_all函数，配合条件变量令所有线程继续执行
    }

    int Push(T val)
    {
        lock_guard<mutex> lock(mutex_);
        if(abort_ == 1)
        {
            return -1;
        }
        queue_.push(val);
        cond_.notify_one();
        return 0;
    }
    int Pop(T &val,const int timeout = 0)
    {
        unique_lock<mutex> lock(mutex_);
        if(queue_.empty())
        {
            //等待push或者超时唤醒
            cond_.wait_for(lock,chrono::milliseconds(timeout),[this]{
                return !queue_.empty() | (abort_ == 1);
            });

        }
        if(abort_ == 1)
        {
            return -1;
        }
        if(queue_.empty())
        {
            return -2;
        }
        val = queue_.front();//获取队列中的值
        queue_.pop();//弹出
        return 0;

    }

    int Front(T &val)
    {
        lock_guard<mutex> lock(mutex_);
        if(abort_ == 1)
        {
            return -1;
        }
        if(queue_.empty())
        {
            return -2;
        }
        val = queue_.front();
        return 0;
    }


    int Size()
    {
        lock_guard<mutex> lock(mutex_);
        return queue_.size();
    }

private:
    int abort_ = 0;//终止符，置1则退出
    mutex mutex_;//互斥锁变量
    condition_variable cond_;//条件变量，满足某条件则阻塞/继续，用于线程同步
    queue<T> queue_;//声明队列容器

};

#endif // QUEUE_H
