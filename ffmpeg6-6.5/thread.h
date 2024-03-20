#ifndef THREAD_H
#define THREAD_H
//线程基类
#include<thread>//c++线程头文件

using namespace std;

class Thread
{
public:
    Thread();
    virtual ~Thread();//析构函数

    virtual int Start();
    virtual int Stop();
    virtual void Run()=0;
protected:
    int abort_ = 0;//此变量置为1则退出
    thread *thread_ = nullptr;//创建一个空线程指针

};

#endif // THREAD_H
