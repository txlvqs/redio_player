#ifndef AVSYNC_H
#define AVSYNC_H

#include <chrono>
#include <ctime>
#include <math.h>
using namespace std::chrono;


class AVSync
{
public:
    AVSync();
    ~AVSync();

    void InitClock(){
        SetClock(NAN);
    }

    //由audio_pts维护
    //初始化设置时钟
    void SetClock(double pts){
        double  time = GetMicroseconds() / 1000000.0;// Convert to seconds //获取当前时间并转换为秒
        pts_drift_ = pts - time; //计算时间戳的漂移
    }

    //由video_pts刷新读取
    double  GetClock()
    {
        double time = GetMicroseconds() / 1000000.0;// Convert to seconds //获取当前时间并转换为秒
        return pts_drift_ + time;
    }


    // 微秒的单位
    time_t GetMicroseconds()
    {
        system_clock::time_point time_point_new = system_clock::now();  //获取当前时间（1970/1/1--今）
        system_clock::duration duration = time_point_new.time_since_epoch();//将获取到的时间转为秒
        time_t us = duration_cast<microseconds>(duration).count();//将转为秒的时间储存在us变量中
        return us;//返回元年至今的秒时间
    }
    double pts_drift_ = 0;

};

#endif // AVSYNC_H
