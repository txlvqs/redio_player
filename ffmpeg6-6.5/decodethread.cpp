#include "decodethread.h"


DecodeThread::DecodeThread(AVPacketQueue *packet_queue, AVFrameQueue *frame_queue):
 packet_queue_(packet_queue),frame_queue_(frame_queue)
{

}

DecodeThread::~DecodeThread()
{

}

int DecodeThread::Init(AVCodecParameters *par)
{

    if(!par){
        printf("DecodeThread::Init par is NULL \n");
        return -1;
    }
    codec_ctx_ = avcodec_alloc_context3(NULL);//分配上下文堆区，上下文就是音频解码器的参数容器，适用格式等数据

    //将传参入的信息结构体中的视频流的参数信息，比如编解码器类型、像素格式、分辨率、帧率等拷贝到处理的结构体中
    int ret = avcodec_parameters_to_context(codec_ctx_,par);
    if(ret < 0)
    {
        av_strerror(ret,err2str,sizeof(err2str));
        printf("avcodec_parameters_to_context failed,ret:%d,err2str:%s",ret,err2str);
        return -1;
    }

    //该结构体指针用于根据上下文中的编码器id字段查找对应的解码器
    const AVCodec *codec = avcodec_find_decoder(codec_ctx_->codec_id);
    if(ret<0)
    {
        av_strerror(ret,err2str,sizeof(err2str));
         printf("avcodec_find_decoder failed,ret:%d,err2str:%s,ret:%d,err2str:%s",ret,err2str);
        return -1;
    }

    //该结构体指针用于使用解码器来解码上下文数据
    ret = avcodec_open2(codec_ctx_,codec,NULL);
    if(ret<0)
    {
         av_strerror(ret,err2str,sizeof(err2str));
         printf("avcodec_open2 failed,ret:%d,err2str:%s",ret,err2str);
         return -1;
    }
    printf("Init decode funish\n");
    return 0;


}

int DecodeThread::Start()
{
    thread_ = new std::thread(&DecodeThread::Run, this); // 为获取文件流再开辟线程
    if(!thread_)
    {
         printf("new DecodeThread failed\n");
         return -1;
    }
    return 0;
}

int DecodeThread::Stop()
{
    printf("DecodeThread::Stop() into\n");
    Thread::Stop();
    return 0;
}

void DecodeThread::Run()
{
    int ret = 0;
    AVFrame *frame = av_frame_alloc();

    while(true)
    {
         if(abort_ == 1)
         {
             break;
         }

         if(frame_queue_->Size() > 10 )//sdfsdfsfdsdf
         {
             std::this_thread::sleep_for(std::chrono::milliseconds(10));
             continue;
         }
         //从paket_queue读取packet
         AVPacket *packet = packet_queue_->Pop(10);//取包，最多等十秒
         if(packet){
             //送给解码器
             ret = avcodec_send_packet(codec_ctx_,packet);
             av_packet_free(&packet);
             if(ret < 0)
             {
                 av_strerror(ret,err2str,sizeof(err2str));
                 printf("avcodec_send_packet failed,ret:%d,err2str:%s",ret,err2str);
                 break;
             }

             //从解码器读取到frame
             while(true){//其中循环是因为有特殊的b帧，需要多次读取才是一个完整的帧
                 ret = avcodec_receive_frame(codec_ctx_,frame);
                 if(ret == 0)
                 {
                     frame_queue_->Push(frame);
//                     printf("%s frame_queue size:%d\n",codec_ctx_->codec->name,frame_queue_->Size());
                     continue;
                 }else if(ret == AVERROR(EAGAIN)){// AVERROR(EAGAIN))该宏表示没有可用帧，将会退出解析
                     break;
                 }
                 else
                 {
                     abort_ = 1;
                     printf("avcodec RUN failed,ret:%d,err2str:%s",ret,err2str);
                     break;
                 }
             }
             //把frame发送给framequeue

         }
         else{
             printf("no packet\n");
         }
    }

}

AVCodecContext *DecodeThread::GetAVCodecContext()
{
    return codec_ctx_;
}
















