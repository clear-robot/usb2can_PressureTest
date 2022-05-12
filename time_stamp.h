//
// Created by kaylor on 2022/4/6.
//

#ifndef SERIAL_PORT__TIME_STAMP_H_
#define SERIAL_PORT__TIME_STAMP_H_

#include <cstdint>

typedef long long timestamp_t;
class TimeStamp {
 public:
  static timestamp_t Now();
  static struct timeval CurrentTime();
  static struct timeval SubTime(struct timeval tv1,struct timeval tv2);
  static struct timeval AddTime(struct timeval tv1,struct timeval tv2);
  static struct timeval MaxTime(struct timeval tv1,struct timeval tv2);
  static struct timeval MinTime(struct timeval tv1,struct timeval tv2);
};
// timestamp_t getnowtime_us();

// struct timeval* add_time(struct timeval *tv1,struct timeval *tv2);
// struct timeval* sud_time(struct timeval *tv1,struct timeval *tv2);

#endif //SERIAL_PORT__TIME_STAMP_H_
