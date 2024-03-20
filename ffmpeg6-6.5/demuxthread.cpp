#include "demuxthread.h"

DemuxThread::DemuxThread(AVPacketQueue * audio_queue,AVPacketQueue * video_queue):
    audio_queue_(audio_queue),video_queue_(video_queue)
{
    printf("DemuxThread Creat\n");
}

DemuxThread::~DemuxThread()
{
    printf("DemuxThread delete\n");
}

int DemuxThread::Init(const char *url)//初始化
{
    if(!url)//如果正确传参文件名
    {
        //调试语句，__FUNCTION__宏为当前函数名，__LINE__为所在函数
        printf("%s(%d)url is null\n",__FUNCTION__,__LINE__);
        return -1;
    }
    if(!audio_queue_ || !video_queue_)
    {
        //调试语句，__FUNCTION__宏为当前函数名，__LINE__为所在函数
        printf("%s(%d)audio_queue_ video_queue_ is null\n",__FUNCTION__,__LINE__);
        return -1;
    }

    url_=url;
    ifmt_ctx_ = avformat_alloc_context();//分配一个上下文结构体空间到本地变量

    int ret = avformat_open_input(&ifmt_ctx_,url_.c_str(),NULL,NULL);//打开音频文件
    if(ret < 0)
    {
       av_strerror(ret,err2str,sizeof(err2str));//将报错编码转为文字信息
       printf("%s(%d)url is null:\n",__FUNCTION__,__LINE__,ret,err2str);//打印报错
        return -1;
    }

    av_dump_format(ifmt_ctx_, 0 ,url_.c_str(),0);//打印文件流信息，用于测试

    //根据文件类型返回音频流索引
    audio_stream_ = av_find_best_stream(ifmt_ctx_,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
    //根据文件类型返回视频流索引
    video_stream_ = av_find_best_stream(ifmt_ctx_,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    //打印扫描结果
    printf("%s(%d)url is null\n，video_stream_:%d,audio_stream_:%d\n",__FUNCTION__,__LINE__,audio_stream_,video_stream_);
    if(audio_stream_< 0 || video_stream_ < 0)
    {
        printf("请打开音视频流项目，行数42\n");
        return -1;
    }
    return 0;
}

int DemuxThread::Start()
{
     thread_ = new std::thread(&DemuxThread::Run, this); // 为获取文件流再开辟线程
    if(!thread_)
     {
        printf("new DemuxThread failed\n");
        return -1;
     }
    return 0;
}

int DemuxThread::Stop()
{
    printf("DemuxThread::Stop() into\n");
    Thread::Stop();
    return 0;
}

//开始读取文件流
void DemuxThread::Run()
{
    printf("DemuxThread::Run() into\n");

    AVPacket packet;//定义一个上下文队列,用于读取文件流
    int ret = 0;

    //在循环中重复读取流数据
    while(1)
    {
        if(abort_ == 1)
        {
            break;
        }
        if(audio_queue_->Size() > 100 || video_queue_->Size() > 100)//wefddddddddddddd
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        ret = av_read_frame(ifmt_ctx_,&packet);//读取流信息语句
        if(ret<0)
        {
            av_strerror(ret,err2str,sizeof(err2str));//将报错编码转为文字信息
            printf("%s(%d)url is null:\n",__FUNCTION__,__LINE__,ret,err2str);//打印报错
            break;
        }

        //检测为音频包队列
        if(packet.stream_index == audio_stream_)//检测为音频包
        {
            audio_queue_->Push(&packet);//将音频包输入音频队列
//            printf("audio pkt size：%d\n",audio_queue_->Size());
        }
        //监测为视频包
        else if(packet.stream_index == video_stream_)//检测为视频包
        {
            video_queue_->Push(&packet);//将视频包输入视频队列
//            printf("vedio pkt size：%d\n",audio_queue_->Size());

        }else {
            av_packet_unref(&packet);//如果都不是，则释放读取的包头
        }

    }

    //关闭文件上下文指示器
    avformat_close_input(&ifmt_ctx_);

    printf("DemuxThread::Run()leave\n");
}

AVCodecParameters *DemuxThread::AudioCodecParameters()
{
    if(audio_stream_ != -1)
    {
        return ifmt_ctx_->streams[audio_stream_]->codecpar;//返回编码器上下文中属于音频流节点的编解码器参数的各种属性，比如编解码器类型、采样率
    }else{
        return NULL;
    }
}

AVCodecParameters *DemuxThread::VideoCodecParameters()
{
    if(video_stream_ != -1)
    {
        return ifmt_ctx_->streams[video_stream_]->codecpar;//返回编码器上下文中属于视频流节点的编解码器参数的各种属性，比如编解码器类型、采样率
    }else{
        return NULL;
    }
}
