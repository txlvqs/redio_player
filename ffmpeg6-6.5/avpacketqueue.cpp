#include "avpacketqueue.h"

AVPacketQueue::AVPacketQueue()
{

}

AVPacketQueue::~AVPacketQueue()
{

}

void AVPacketQueue::Abort()
{
    //需要注意的是，即使是关闭也需要先release把所有数据到干净才可以关闭
    release();
    queue_.Abort();
}

int AVPacketQueue::Size()
{
    return queue_.Size();
}

int AVPacketQueue::Push(AVPacket *val)
{
    AVPacket *tmp_pkt = av_packet_alloc();//av_packet_alloc是动态分配音视频包空间的函数
    av_packet_move_ref(tmp_pkt,val);//将参数拷贝到本地指针进行操作
    return queue_.Push(tmp_pkt);
}

AVPacket *AVPacketQueue::Pop(const int timeout)
{
    AVPacket *tmp_pkt = NULL;
    //"这里的时间点作为音乐视频的人标记点，如果不主动跳转播放则会在规定时间点pop出播放"
    int ret = queue_.Pop(tmp_pkt,timeout);//pop函数将会将消息队列中的消息复制给tmp_pkt,并且消息队列弹出一格

    if(ret<0)
    {
        if(ret == -1)
        {
            printf("queue_ abort\n");
        }
    }

    return tmp_pkt;//返回消息队列的消息
}

void AVPacketQueue::release()
{
    while(1)
    {
        AVPacket *tmp_pkt = NULL;
        int ret = queue_.Pop(tmp_pkt,1);//pop函数将会将消息队列中的消息复制给tmp_pkt,并且消息队列弹出一格
        if(ret<0)
        {
            break;
        }else
        {
            av_packet_free(&tmp_pkt);//释放前面分配给AVPacket的堆区空间
            continue;
        }
    }
}
