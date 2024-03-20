#ifndef AVFRAMEQUEUE_H
#define AVFRAMEQUEUE_H

#include "queue.h"

#ifdef __cplusplus//在c++环境下包含c格式头文件
extern "C"
{
#include"libavcodec/avcodec.h"
}
#endif


class AVFrameQueue
{
public:
    AVFrameQueue();
    ~AVFrameQueue();
    void Abort();//退出函数
    int Size();//数据长度
    int Push(AVFrame *val);//AVPacket是ffmepg的内置结构体，为解码包结构
    AVFrame *Pop(const int timeout);//传参时间点，根据时间点决定是否出列数据
    //"这里的时间点作为音乐视频的人标记点，如果不主动跳转播放则会在规定时间点pop出播放"
    AVFrame *Front();

private:
    void release();//发布"解析到的数据将交给播放函数处理"
    Queue<AVFrame *>queue_;//声明一个 Queue模板AV包对象作为私有变量
};

#endif // AVFRAMEQUEUE_H
