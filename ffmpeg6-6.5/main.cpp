#include <iostream>
#include "demuxthread.h"
#include "decodethread.h"
#include "audiooutput.h"
#include "videooutput.h"
#include  "avsync.h"
using namespace std;
#undef main
// 包含ffmpeg头文件
//#include "libavutil/avutil.h"

int main(int argc,char* argv[])
{
    printf("url :%s\n",argv[1]);

    int ret = 0;
    AVPacketQueue audio_packet_queue;//提取的音频包
    AVPacketQueue video_packet_queue;//提取的视频包
    AVFrameQueue audio_frame_queue;//解析的音频包
    AVFrameQueue video_frame_queue;//解析的视频包
    AVSync avsync;//同步时钟

    DemuxThread *demux_thread = new DemuxThread(&audio_packet_queue,&video_packet_queue);//开辟一个分析流线程指针对象(解析出来的数据将临时存放在这里）

    //传入文件名参数进行初始化线程
    ret = demux_thread->Init(argv[1]);
    if(ret < 0)
    {
        printf("%s(%d)url is demux_thread->Init\n",__FUNCTION__,__LINE__);//打印报错
        return -1;
    }
    //随后调用用线程进行循环读取文件线程
    ret = demux_thread->Start();
    if(ret < 0)
    {
        printf("%s(%d)url is demux_thread->Start\n",__FUNCTION__,__LINE__);//打印报错
        return -1;
    }

    //音频解码流程
    DecodeThread *audio_decode_thread = new  DecodeThread(&audio_packet_queue,&audio_frame_queue);
    ret = audio_decode_thread->Init(demux_thread->AudioCodecParameters());//初始化
    if(ret < 0)
    {
        printf("%s(%d)audio_decode_thread Init\n",__FUNCTION__,__LINE__);//打印报错
        return -1;
    }
    ret = audio_decode_thread->Start();//解析
    if(ret < 0)
    {
        printf("%s(%d)audio_decode_thread Start\n",__FUNCTION__,__LINE__);//打印报错
        return -1;
    }


    //视频解码流程
    DecodeThread *video_decode_thread = new  DecodeThread(&video_packet_queue,&video_frame_queue);
    ret = video_decode_thread->Init(demux_thread->VideoCodecParameters());//初始化
    if(ret < 0)
    {
        printf("%s(%d)video_decode_thread Init\n",__FUNCTION__,__LINE__);//打印报错
        return -1;
    }
    ret = video_decode_thread->Start();//解析
    if(ret < 0)
    {
        printf("%s(%d)video_decode_thread Start\n",__FUNCTION__,__LINE__);//打印报错
        return -1;
    }

    //同步时钟初始化
    avsync.InitClock();


    AudioParams audio_params;
    memset(&audio_params,0,sizeof(audio_params));
    audio_params.ch_layout = audio_decode_thread->GetAVCodecContext()->ch_layout;
    audio_params.fmt=audio_decode_thread->GetAVCodecContext()->sample_fmt;
    audio_params.freq = audio_decode_thread->GetAVCodecContext()->sample_rate;

    AudioOutPut *audio_output = new AudioOutPut(&avsync,audio_params,&audio_frame_queue,demux_thread->AudioStreamTimebase());
    ret =  audio_output->Init();
    if(ret < 0)
    {
        printf("%s(%d)audio_output->Init() fail\n",__FUNCTION__,__LINE__);//打印报错
        return -1;
    }


    //播放视频
    //创建视频框对象
    VideoOutput *video_output_ = new VideoOutput(&avsync,&video_frame_queue,video_decode_thread->GetAVCodecContext()->width,
                                                 video_decode_thread->GetAVCodecContext()->height,demux_thread->VideoStreamTimebase());
    //初始化视频播放工具
    ret = video_output_->Init();
    if(ret < 0)
    {
        printf("%s(%d video_output_->Init(); fail\n",__FUNCTION__,__LINE__);//打印报错
        return -1;
    }
    //创建视频播放
    video_output_->MainLoop();


    //    //随眠一段时间
    //    std::this_thread::sleep_for(std::chrono::milliseconds(30000));
    demux_thread->Stop();//退出两个线程
    delete demux_thread;//释放线程指针
    printf("%s(%d)decode_thread Start\n",__FUNCTION__,__LINE__);//打印报错

    audio_decode_thread->Stop();//退出音频线程
    delete audio_decode_thread;//释放音频指针
    printf("%s(%d)audio_decode_thread Start\n",__FUNCTION__,__LINE__);//打印报错

    video_decode_thread->Stop();//退出视频线程
    delete video_decode_thread;//释放视频指针
    printf("%s(%d)video_decode_thread Start\n",__FUNCTION__,__LINE__);//打印报错

    audio_output->DeInit();//清理关闭硬件音响
    delete audio_output;//释放硬件音响指针


    printf("main finish");
    return 0;
}
