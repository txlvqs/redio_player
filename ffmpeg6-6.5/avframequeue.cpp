#include "avframequeue.h"

AVFrameQueue::AVFrameQueue()
{

}

AVFrameQueue::~AVFrameQueue()
{

}

void AVFrameQueue::Abort()
{
    //需要注意的是，即使是关闭也需要先release把所有数据到干净才可以关闭
    release();
    queue_.Abort();
}

int AVFrameQueue::Size()
{
    return queue_.Size();
}

int AVFrameQueue::Push(AVFrame *val)
{
    AVFrame *tmp_frame = av_frame_alloc();//av_frame_alloc是动态分配音视频包空间的函数
    av_frame_move_ref(tmp_frame,val);//将参数拷贝到本地指针进行操作
    return queue_.Push(tmp_frame);
}

AVFrame *AVFrameQueue::Pop(const int timeout)
{
    AVFrame *tmp_frame = NULL;
    //"这里的时间点作为音乐视频的人标记点，如果不主动跳转播放则会在规定时间点pop出播放"
    int ret = queue_.Pop(tmp_frame,timeout);//pop函数将会将消息队列中的消息复制给tmp_pkt,并且消息队列弹出一格

    if(ret<0)
    {
        if(ret == -1)
        {
            printf("queue_ abort\n");
        }
    }

    return tmp_frame;//返回消息队列的消息
}

AVFrame *AVFrameQueue::Front()
{
     AVFrame *tmp_frame = NULL;
    int ret = queue_.Front(tmp_frame);
     if(ret<0){
        if(ret == -1)
        {
            printf("queue_abort\n");
        }
    }
     return tmp_frame;
}

void AVFrameQueue::release()
{

     while(1)
     {
        AVFrame *tmp_frame = NULL;
        int ret = queue_.Pop(tmp_frame,1);//pop函数将会将消息队列中的消息复制给tmp_pkt,并且消息队列弹出一格
        if(ret<0)
        {
            break;
        }else
        {
            av_frame_free(&tmp_frame);//释放前面分配给AVFrame的堆区空间
            continue;
        }
     }
}
