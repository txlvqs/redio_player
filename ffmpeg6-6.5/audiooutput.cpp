#include "audiooutput.h"

AudioOutPut::AudioOutPut(AVSync *avsync,const AudioParams &audio_params,AVFrameQueue *frame_queue,AVRational time_base):
    avsync_(avsync),frame_queue_(frame_queue),src_tgt_(audio_params),time_base_(time_base)
{

}

AudioOutPut::~AudioOutPut()
{

}

void sdl_audio_callback(void *userdata, Uint8 *stream, int len) // 定义了一个名为sdl_audio_callback的回调函数
{
    AudioOutPut *audio_output = (AudioOutPut *)userdata; // 将userdata强制转换为AudioOutPut指针
//    printf("sdl_audio_callback_len:%d\n",len);
    while(len>0)
    {
        if(audio_output->audio_buf_index == audio_output->audio_buf_size){//标识当前时间节点没达到最大值，还可以继续播放
            //1. 读取pcm的数据
            audio_output->audio_buf_index = 0;
            AVFrame *frame = audio_output->frame_queue_->Pop(2);

            //这行代码的作用是将帧（frame）的显示时间戳（pts）转换为秒，并将结果赋值给音频输出对象（audio_output）的 pts 属性
            audio_output->pts = frame->pts * av_q2d(audio_output->time_base_);//时间戳:表示该帧在整组视频中的第几秒播放


            if(frame)
            {
                //2.  作重采样

                //2.1 初始化采样器
                if((//检测三样属性全部与Init中设置的属性一样才可以解析，否则需要转码重新采样
                        (frame->format != audio_output->dst_tgt_.fmt) //采样格式判断
                        ||(frame->sample_rate != audio_output->dst_tgt_.freq) // 采样率相关
                        ||av_channel_layout_compare(&frame->ch_layout,&audio_output->dst_tgt_.ch_layout) != 0)//比较通道数量是否相同
                    &&(!audio_output->swr_ctx_)){//采样器是否开启，没开启也需要在重采样模块中再次开启采样器

                    swr_alloc_set_opts2(&audio_output->swr_ctx_, // 配置音频重采样参数的目标SwrContext结构体指针
                                        &audio_output->dst_tgt_.ch_layout, // 目标音频的通道布局
                                        audio_output->dst_tgt_.fmt, // 目标音频的采样格式
                                        audio_output->dst_tgt_.freq, // 目标音频的采样率
                                        &frame->ch_layout, // 输入音频帧的通道布局
                                        (enum AVSampleFormat)frame->format, // 输入音频帧的采样格式
                                        frame->sample_rate, // 输入音频帧的采样率
                                        0, // 重采样的选项
                                        NULL); // 额外的参数

                    //检测搭采样器是否初始化，如果未初始化或者采样器初始化失败则释放采样器，终止当前操作
                    if(!audio_output->swr_ctx_||swr_init(audio_output->swr_ctx_)<0)
                    {
                        printf("swr_init failed");
                        if(audio_output->swr_ctx_)
                        {
                            swr_free(&audio_output->swr_ctx_);
                        }
                        return;
                    }
                }
                if(audio_output->swr_ctx_){
                    //需要重新采样
                    const uint8_t **in = (const uint8_t **)frame->extended_data;//将音频数据传入输入端
                    uint8_t **out = (uint8_t **)&audio_output->audio_buf1_;//将输出口分配空间

                    //该行将计算输出音频帧的采样个数，并添加额外缓冲区256，计算方式:输入采样个数*输入采样率/目标采样率+256缓冲区
                    int out_samples = frame->nb_samples * audio_output->dst_tgt_.freq / frame->sample_rate + 256;

                    // 计算输出音频数据的字节数
                    int out_bytes = av_samples_get_buffer_size(
                        NULL,  // 不需要特定的对齐方式
                        audio_output->dst_tgt_.ch_layout.nb_channels,  // 输出音频的通道数量
                        out_samples,  // 输出音频的采样个数
                        audio_output->dst_tgt_.fmt,  // 输出音频的采样格式
                        0  // 不需要特定的对齐方式
                        );
                    if(out_bytes < 0)
                    {
                        printf("av_samples_get_buffer_size_failed");
                        return;
                    }
                    //为本地空间audio_buf1_分配大小，分配audio_buf1_size指定的大小，out_bytes为需要分配的字节数
                    //当指定大小小于需要分配大小则会分配out_bytes的大小
                    av_fast_malloc(&audio_output->audio_buf1_,&audio_output->audio_buf1_size,out_bytes);

                    //swr_convert函数将输入音频数据in转换为输出音频数据out，并返回转换后的采样个数len2。
                    //参数列表（采样器指针，输出指针，输出采样个数，输入指针，输入采样个数）
                    int len2 = swr_convert(audio_output->swr_ctx_,out,out_samples,in,frame->nb_samples);
                    if(len2<0)
                    {
                        printf("swr_convert failed\n");
                        return;
                    }
                    //计算出具体需要的采样空间大小
                    audio_output->audio_buf_size =av_samples_get_buffer_size(
                        NULL,  // 不需要特定的对齐方式
                        audio_output->dst_tgt_.ch_layout.nb_channels,  // 输出音频的通道数量
                        len2,  // 输出音频的采样个数
                        audio_output->dst_tgt_.fmt,  // 输出音频的采样格式
                        0  // 不需要特定的对齐方式
                        );

                    //最终将数据从预留空间传入目标空间
                    audio_output->audio_buf_ = audio_output->audio_buf1_;

                } else {//不需要重采样
                    int out_bytes = av_samples_get_buffer_size(
                        NULL,  // 不需要特定的对齐方式，传入NULL
                        frame->ch_layout.nb_channels,  // 输出音频的通道数量
                        frame->nb_samples,  // 输出音频的采样个数
                        (enum AVSampleFormat)frame->format,  // 输出音频的采样格式
                        0  // 不需要特定的对齐方式
                        );  // 计算输出音频数据的字节数

                    av_fast_malloc(&audio_output->audio_buf1_, &audio_output->audio_buf1_size, out_bytes);  // 为输出音频数据分配内存空间
                    audio_output->audio_buf_ = audio_output->audio_buf1_;  // 将audio_buf1_指针赋值给audio_buf_
                    audio_output->audio_buf1_size = out_bytes;  // 设置audio_buf1_size为输出音频数据的字节数
                    memcpy(audio_output->audio_buf_, frame->extended_data[0], out_bytes);  // 将输入音频数据复制到输出音频数据的缓冲区中
                }
                av_frame_free(&frame);
            } else{//读取不到frame数据
                audio_output->audio_buf_ = NULL;
                audio_output->audio_buf_size = 512;

            }

        }//end of if(audio_output->audio_buf_index<audio_output->audio_buf_size)



        // 3.拷贝数据到stream buffer
        int len3 = audio_output->audio_buf_size - audio_output->audio_buf_index;  // 计算剩余未处理的音频数据大小
        //每拷贝出去一份则len-=xx
        if(len3 > len) {
            len3 = len;  // 如果剩余数据大于需要拷贝的数据，将len3设置为len
        }
        if(!audio_output->audio_buf_) {
            memset(stream, 0, len3);  // 如果audio_buf_为空，将stream缓冲区清零，长度为len3
        } else {
            memcpy(stream, audio_output->audio_buf_ + audio_output->audio_buf_index, len3);  // 如果audio_buf_不为空，将其中的数据拷贝到stream缓冲区中，长度为len3
        }
        len -= len3;  // 更新len，表示已经处理的数据大小
        audio_output->audio_buf_index += len3;  // 更新audio_buf_index，表示已处理的数据大小
        stream += len3;  // 更新stream指针，指向未填充数据的位置

        printf("len:%d,sudio_buf_index:%d,%d\n",len,audio_output->audio_buf_index,audio_output->audio_buf_size);

    }
    //调用同步时钟方法，通过音频时间戳来设置同步时钟
//    printf("audio pts: %0.3lf\n",audio_output->pts);
    audio_output->avsync_->SetClock(audio_output->pts);
}


int AudioOutPut::Init() // AudioOutput类的初始化函数
{
    if(SDL_Init(SDL_INIT_AUDIO) != 0) // 初始化SDL音频子系统
    {
        printf("SDL_init failed\n");
        return -1;
    }
    SDL_AudioSpec wanted_spec; // 创建一个SDL_AudioSpec结构wanted_spec,SDL_AudioSpec是SDL内置结构体表示文件流的一些属性数据
    wanted_spec.channels = 2; // 设置音频的通道数为2
    wanted_spec.freq = src_tgt_.freq; // 设置音频的采样率
    wanted_spec.format = AUDIO_S16SYS; // 设置音频的格式为16位PCM
    wanted_spec.silence = 0; // 设置静音值为0
    wanted_spec.callback = sdl_audio_callback; // 设置回调函数
    wanted_spec.userdata = this; // 将this指针作为userdata传递给SDL
    wanted_spec.samples = 1024;//每1024个样本回调一次  总单次提取数据：2（一次提取字节数）*2（通道数）*1024（每1024回调一次）

    int ret = SDL_OpenAudio(&wanted_spec,NULL);//根据上面结构体参数的方式打开硬件音频设备准备播放
    if(ret != 0)
    {
        printf("SDL_OpenAudio failed\n");
        return -1;
    }

    av_channel_layout_default(&dst_tgt_.ch_layout, wanted_spec.channels); // 设置目标音频的通道布局
    dst_tgt_.fmt = AV_SAMPLE_FMT_S16; // 设置目标音频的格式为16位整数型
    dst_tgt_.freq = wanted_spec.freq; // 设置目标音频的采样率为所需的采样率
    SDL_PauseAudio(0); // 恢复音频播放，开始播放音频数据
    printf("AudioOutPUT::init() finish\n");



    return 0;
}

int AudioOutPut::DeInit()
{
    SDL_PauseAudio(1);//暂停播放
    SDL_CloseAudio();//关闭硬件音频
     return 0;
}
