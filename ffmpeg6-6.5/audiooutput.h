#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include "avframequeue.h"
#include "avsync.h"
#ifdef __cplusplus//在c++环境下包含c格式头文件
extern "C"
{
#include "SDL.h"
#include "libswresample/swresample.h"
}
#endif

typedef struct _AudioParams
{
    int freq;//采样率
    AVChannelLayout ch_layout;//通道布局
    enum AVSampleFormat fmt;// 采样格式
}AudioParams;


class AudioOutPut
{
public:
    AudioOutPut(AVSync *avsync,const AudioParams &audio_params,AVFrameQueue *frame_queue,AVRational time_base);
    ~AudioOutPut();
    int Init();
    int DeInit();

public:
    AVFrameQueue *frame_queue_ = NULL;
    AudioParams src_tgt_;//解码后的源pcm格式
    AudioParams dst_tgt_;//SDL需要的格式

    struct SwrContext *swr_ctx_ = NULL;//SwrContext是ffmepg内置的用于重采样结构体

    uint8_t *audio_buf1_ = NULL;
    uint32_t audio_buf1_size = 0;//真正分配空间大小  需要注意的是audio_buf_size<=audio_buf1_size才能正常运行

    uint8_t *audio_buf_ = NULL;
    uint32_t audio_buf_size = 0;//真正采样率大小

    uint32_t audio_buf_index = 0;//标识采样实际位置

    AVRational time_base_;
    AVSync *avsync_ = NULL;

    double pts = 0;



};

#endif // AUDIOOUTPUT_H
