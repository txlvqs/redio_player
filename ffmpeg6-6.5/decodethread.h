#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include "thread.h"
#include "avpacketqueue.h"
#include "avframequeue.h"



class DecodeThread : public Thread
{
public:
    DecodeThread(AVPacketQueue *packet_queue,AVFrameQueue *frame_queue);
    ~DecodeThread();
    int Init(AVCodecParameters *par);
    int Start();
    int Stop();
    void Run();
    AVCodecContext *GetAVCodecContext();
private:
    char err2str[256] = {0};
    AVCodecContext * codec_ctx_ = NULL;//编码器上下文结构体
    AVPacketQueue *packet_queue_ = NULL;//包结构指针对象
    AVFrameQueue *frame_queue_ = NULL;//解码包解构指针对想


};

#endif // DECODETHREAD_H
