#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H

#include<iostream>
#include "thread.h"
#include "avpacketqueue.h"

#ifdef __cplusplus//在c++环境下包含c格式头文件
extern "C"
{
#include"libavutil/avutil.h"
#include"libavformat/avformat.h"
}
#endif


class DemuxThread : public Thread
{
public:
    DemuxThread(AVPacketQueue * audio_queue,AVPacketQueue * video_queue);
    virtual  ~DemuxThread();

    virtual int Init(const char *url);//初始化文件函数，外部传参媒体文件名
    virtual int Start();
    virtual int Stop();
    virtual void Run();//开始读取文件流信息

    AVCodecParameters *AudioCodecParameters();
    AVCodecParameters *VideoCodecParameters();

    AVRational AudioStreamTimebase()
    {
        return ifmt_ctx_->streams[audio_stream_]->time_base;
    }
    AVRational VideoStreamTimebase()
    {
        return ifmt_ctx_->streams[video_stream_]->time_base;
    }

private:
    string url_;
    AVFormatContext *ifmt_ctx_ =NULL;
    char err2str[256];

    //流索引：用于定位视频帧与音频帧
    int audio_stream_ = -1;//音频流索引
    int video_stream_ = -1;//视频流索引
    AVPacketQueue * audio_queue_ = NULL;
    AVPacketQueue * video_queue_ = NULL;

};

#endif // DEMUXTHREAD_H
