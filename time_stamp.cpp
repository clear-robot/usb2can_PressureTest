//
// Created by kaylor on 2022/4/6.
//

#include "time_stamp.h"
#include <sys/time.h>
#include <stdio.h>
#include <bits/types.h>
#include <cstdint>

timestamp_t TimeStamp::Now()
{
  struct timeval tv;
  gettimeofday(&tv,0);
  //timestamp_t time=(timestamp_t)tv.tv_sec*1000+tv.tv_usec/1000;  //ms
  timestamp_t time=(timestamp_t)tv.tv_sec*1000000 + tv.tv_usec;  //us
  return time;
}

  struct timeval TimeStamp::CurrentTime(){
    struct timeval tv;
    gettimeofday(&tv,0);
    return tv;
  }

  struct timeval TimeStamp::AddTime(struct timeval tv1,struct timeval tv2){
    struct timeval tv;
    tv.tv_usec = tv1.tv_usec + tv2.tv_usec;
    if (tv.tv_usec >= 1000000){
      tv.tv_sec = tv1.tv_sec + tv2.tv_sec + 1;
      tv.tv_usec -= 1000000;
    }else{
      tv.tv_sec = tv1.tv_sec + tv2.tv_sec;
    }
    return tv;
  }
  struct timeval TimeStamp::SubTime(struct timeval tv1,struct timeval tv2){
    struct timeval tv;
    if (tv1.tv_sec > tv2.tv_sec){
      if (tv1.tv_usec - tv2.tv_usec < 0){
        tv.tv_usec = tv1.tv_usec + 1000000 - tv2.tv_usec;
        tv.tv_sec = tv1.tv_sec - tv2.tv_sec - 1;
      }else{
        tv.tv_usec = tv1.tv_usec - tv2.tv_usec;
        tv.tv_sec = tv1.tv_sec - tv2.tv_sec;
      }
    }else if(tv1.tv_sec == tv2.tv_sec){
      if(tv1.tv_usec > tv2.tv_usec){
        tv.tv_sec = 0;
        tv.tv_usec = tv1.tv_usec - tv2.tv_usec;
      }else{
        tv.tv_sec = 0;
        tv.tv_usec = tv2.tv_usec - tv1.tv_usec;
      }
    }
    else{
      if (tv2.tv_usec - tv1.tv_usec < 0){
        tv.tv_usec = tv2.tv_usec + 1000000 - tv1.tv_usec;
        tv.tv_sec = tv2.tv_sec - tv1.tv_sec - 1;
      }else{
        tv.tv_usec = tv2.tv_usec - tv1.tv_usec;
        tv.tv_sec = tv2.tv_sec - tv1.tv_sec;
      }
    }
    //printf("tv.sec = %ld\tv.usec = %ld\n",tv.tv_sec,tv.tv_usec);
    return tv;
  }

  struct timeval TimeStamp::MaxTime(struct timeval tv1,struct timeval tv2){
    struct timeval tv;
    if (tv1.tv_sec > tv2.tv_sec){
      tv.tv_sec = tv1.tv_sec;
      tv.tv_usec = tv1.tv_usec;
    }else if (tv1.tv_sec == tv2.tv_sec && tv1.tv_usec > tv2.tv_usec){
      tv.tv_sec = tv1.tv_sec;
      tv.tv_usec = tv1.tv_usec;
    }else{
      tv.tv_sec = tv2.tv_sec;
      tv.tv_usec = tv2.tv_usec;
    }
    return tv;
  }

  struct timeval TimeStamp::MinTime(struct timeval tv1,struct timeval tv2){
    struct timeval tv;
    if (tv1.tv_sec < tv2.tv_sec){
      tv.tv_sec = tv1.tv_sec;
      tv.tv_usec = tv1.tv_usec;
    }else if (tv1.tv_sec == tv2.tv_sec && tv1.tv_usec < tv2.tv_usec){
      tv.tv_sec = tv1.tv_sec;
      tv.tv_usec = tv1.tv_usec;
    }else{
      tv.tv_sec = tv2.tv_sec;
      tv.tv_usec = tv2.tv_usec;
    }
    return tv;
  }