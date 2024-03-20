#include "thread.h"

Thread::Thread()
{

}

Thread::~Thread()
{

}

int Thread::Start()
{
    return 0;
}

int Thread::Stop()
{
    abort_ = 1;//置为1则退出，保护成员仅父类可操作
    //如果线程存在
    if(thread_)
    {
        thread_->join();//阻塞等待线程执行完毕
        delete thread_;//随后关闭线程
        thread_ = nullptr;
    }
    return 0;
}
