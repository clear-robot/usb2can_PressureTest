#include <iostream>
#include <cstring>
#include <unistd.h>
#include "ptz_control.h"
#include <string.h>
#include <sys/time.h>
#include "time_stamp.h"
struct CAN_Fream                   //can发送功能相关结构体  16bytes
{
    uint8_t freamHeader;        //发送标志位 0x55
    uint8_t CMD;                //CAN 命令  //00 转发数据  0x01  设置波特率  0x02 接收数据  0x03 上传心跳数据  0x04 请求设备心跳  0x10 can接收回传
    uint8_t subCmd;             //CAN
    uint8_t freamLen;           //帧长
    uint32_t CTRLID;              //控制段
    uint8_t CANPort;            //can 端口
    uint8_t canDataLen;         //数据长度
    uint8_t canData[8];         //Can 数据
    uint8_t freamEnd;           //结尾       0xaa

};
struct CAN_Fream send_frame = {0x55, 0x00, 0x01, 0x13, 0x00, 0x01, 0x08};
struct CAN_Fream recv_control_frame = {0x55, 0x00, 0x01, 0x13, 0x00, 0x00, 0x08};

void frame_data_cmp();
void count_sendrecv_dalay();
void frame_sendThread_data_cmp();
int main() {
  std::cout << "time = " << TimeStamp::Now() << std::endl;

  std::cout << "sizeof(long) = " << sizeof(long) << std::endl;

  // Send port
  SerialPort send_control;
  if (send_control.open_send_port("/dev/ttyACM3", 9600, 8, SerialPort::PARITY_NONE, 1) != SerialPort::OK){
        std::cout << "Can not open send serial port" << std::endl;
        return -1;

  }

  // Receive port
  SerialPort recv_control;
  if (recv_control.Open("/dev/ttyACM0", 9600, 8, SerialPort::PARITY_NONE, 1) != SerialPort::OK){
    std::cout << "Can not open recv serial port" << std::endl;
    return -1;
  }

  send_frame.freamEnd = 0xAA;
  recv_control_frame.freamEnd = 0xAA;

  struct timeval send_first_package_time;
  struct timeval send_last_package_time;
  gettimeofday(&send_first_package_time,0);

  uint8_t send_control_buf[100] = {0};
  uint8_t recv_control_buf[100] = {0};
  uint32_t send_package_fail_number = 0;
  uint32_t recv_control_package_fail_number = 0;
  unsigned long long int send_package_number = 0x1; //发送包，计数+1

  while (send_package_number <= SET_SEND_PACKAGE_NUMBER){
      // Set send number to canData
      
      memcpy(send_frame.canData, &send_package_number, sizeof(send_frame.canData));
      // Set send package data
      memcpy(send_control_buf, &send_frame, sizeof(send_frame));
      gettimeofday(&send_fream_list[send_package_number].time,0);
      if (send_control.Write((char*)&send_control_buf[0], 19) <= 0){
          send_package_fail_number++;
          continue;
      }

      
      memcpy(recv_control_frame.canData, &send_package_number, sizeof(recv_control_frame.canData));
      memcpy(recv_control_buf, &recv_control_frame, sizeof(recv_control_frame));

      gettimeofday(&recvcontol_send_fream_list[send_package_number].time,0);
      if (recv_control.Write((char*)&recv_control_buf[0], 19) <= 0){
          recv_control_package_fail_number++;
          continue;
      }
            // printf("=======");
            // for (size_t i = 0; i < 19; i++)
            // {
            //     printf("%02X ", recv_control_buf[i]);
            // }
            // printf("\n");

      // Save the mask to send_list
      memcpy(send_fream_list[send_package_number].canData, send_frame.canData, sizeof(send_frame.canData));

      memcpy(recvcontol_send_fream_list[send_package_number].canData, recv_control_frame.canData, sizeof(recv_control_frame.canData));

      send_package_number++;
      // usleep(150);  //us

  }
  gettimeofday(&send_last_package_time,0);
  printf("all bytes:%d\n",19*SET_SEND_PACKAGE_NUMBER);
  struct timeval alltime = TimeStamp::SubTime(send_first_package_time,send_last_package_time);
  long long send_alltime = alltime.tv_sec*1000000 + alltime.tv_usec;
  printf("alltime =  %lld \n",send_alltime);
  printf("Speed:%f byte/s\n", 19*SET_SEND_PACKAGE_NUMBER/((float)send_alltime/1000/1000));

  frame_data_cmp();
  frame_sendThread_data_cmp();
  std::cout << "exit" << std::endl;
  return 0;
}
#if 1
bool cmp_canData(uint8_t *s1,uint8_t *s2){
  for(int i = 0;i < 8;i++){
    // printf("%x   %x\n",s1[i],s2[i]);
    if(s1[i] != s2[i]){
      return false;
    }
  }
  return true;
}
void frame_data_cmp(){
  struct timeval all_sendrecv_dalay = {0};
  struct timeval MAX_sendrecv_delay = {0};
  struct timeval MIN_sendrecv_delay;
  MIN_sendrecv_delay.tv_usec = 2147483648;  // 2^31
  struct timeval temp_delay;
  // average_sandrecv_delay = {0};
  long long recv_number = 0;
//problem should be find
  if(receive_package_number != SET_SEND_PACKAGE_NUMBER){
     printf("receive_package_number = %d\n",receive_package_number);
     return;
  }
  int i = 1;
  int j = 1;

  for (i = 1;i <= SET_SEND_PACKAGE_NUMBER;i++){
    j = i;
      for (;j <= SET_SEND_PACKAGE_NUMBER;j++){
          if (cmp_canData(recv_fream_list[i].canData,send_fream_list[j].canData) == true){
              temp_delay = TimeStamp::SubTime(recv_fream_list[i].time,send_fream_list[j].time);
              recv_number++;
              // printf("temp_delay = %ld  recv_fream_list[%d].time = %ld - send_fream_list[%d].time = %ld \n",
              //                                                                          temp_delay.tv_sec*1000000 + temp_delay.tv_usec,
              //                                                                          i,recv_fream_list[i].time.tv_sec*1000000 + recv_fream_list[i].time.tv_usec,
              //                                                                          j,send_fream_list[j].time.tv_sec*1000000 + send_fream_list[j].time.tv_usec);
              //printf("%d  temp_dalay = %d\n",i,temp_dalay);

              all_sendrecv_dalay = TimeStamp::AddTime(all_sendrecv_dalay,temp_delay);
              //printf("all_sendrecv_dalay = %d\n",all_sendrecv_dalay);
              MAX_sendrecv_delay = TimeStamp::MaxTime(MAX_sendrecv_delay,temp_delay);
              MIN_sendrecv_delay = TimeStamp::MinTime(MIN_sendrecv_delay,temp_delay);
             
              break;
          }else{
            std::cout << "Erorr: i = " << i  <<" j = "<< j <<"\n"; 
          }
      }
  }
  // average_sandrecv_delay = all_sendrecv_dalay / recv_number;
  printf("\nall_sendrecv_dalay = %lld\n",(long long)all_sendrecv_dalay.tv_sec*1000000 + (long long)all_sendrecv_dalay.tv_usec);
  // printf("MAX_sendrecv_delay = %lld\n",(long long)MAX_sendrecv_delay.tv_sec*1000000 + (long long)MAX_sendrecv_delay.tv_usec);
  // printf("MIN_sendrecv_delay = %lld\n",(long long)MIN_sendrecv_delay.tv_sec*1000000 + (long long)MIN_sendrecv_delay.tv_usec);
  printf("recv_number = %lld\n",recv_number);
  printf("average_sandrecv_delay = %lld\n",((long long)all_sendrecv_dalay.tv_sec*1000000 + (long long)all_sendrecv_dalay.tv_usec)/SET_SEND_PACKAGE_NUMBER);

}

void frame_sendThread_data_cmp(){
  struct timeval all_sendrecv_dalay = {0};
  struct timeval MAX_sendrecv_delay = {0};
  struct timeval MIN_sendrecv_delay;
  MIN_sendrecv_delay.tv_usec = 2147483648;  // 2^31
  struct timeval temp_delay;
  // average_sandrecv_delay = {0};
  long long recv_number = 0;
//problem should be find
  if(receive_package_number_SendThread != SET_SEND_PACKAGE_NUMBER){
     printf("===receive_package_number_SendThread = %d\n",receive_package_number_SendThread);
     return;
  }
  int i = 1;
  int j = 1;

  for (i = 1;i <= SET_SEND_PACKAGE_NUMBER;i++){
    j = i;
      for (;j <= SET_SEND_PACKAGE_NUMBER;j++){
          if (cmp_canData(recv_fream_list_sendThread[i].canData,recvcontol_send_fream_list[j].canData) == true){

              temp_delay = TimeStamp::SubTime(recv_fream_list_sendThread[i].time,recvcontol_send_fream_list[j].time);
              
              recv_number++;
              // printf("temp_delay = %ld  recv_fream_list_sendThread[%d].time = %ld - recvcontol_send_fream_list[%d].time = %ld \n",
              //                                                                          temp_delay.tv_sec*1000000 + temp_delay.tv_usec,
              //                                                                          i,recv_fream_list_sendThread[i].time.tv_sec*1000000 + recv_fream_list_sendThread[i].time.tv_usec,
              //                                                                          j,recvcontol_send_fream_list[j].time.tv_sec*1000000 + recvcontol_send_fream_list[j].time.tv_usec);
              // printf("%d  temp_dalay = %d\n",i,temp_delay);

              all_sendrecv_dalay = TimeStamp::AddTime(all_sendrecv_dalay,temp_delay);
              MAX_sendrecv_delay = TimeStamp::MaxTime(MAX_sendrecv_delay,temp_delay);
              MIN_sendrecv_delay = TimeStamp::MinTime(MIN_sendrecv_delay,temp_delay);
             
              break;
          }else{
            std::cout << "Erorr: i = " << i  <<" j = "<< j <<"\n"; 
          }
      }
  }
  printf("\nsendThread all_sendrecv_dalay = %lld\n",(long long)all_sendrecv_dalay.tv_sec*1000000 + (long long)all_sendrecv_dalay.tv_usec);
  // printf("sendThread MAX_sendrecv_delay = %lld\n",(long long)MAX_sendrecv_delay.tv_sec*1000000 + (long long)MAX_sendrecv_delay.tv_usec);
  // printf("sendThread MIN_sendrecv_delay = %lld\n",(long long)MIN_sendrecv_delay.tv_sec*1000000 + (long long)MIN_sendrecv_delay.tv_usec);
  printf("sendThread recv_number = %lld\n",recv_number);
  printf("sendThread average_sandrecv_delay = %lld\n",((long long)all_sendrecv_dalay.tv_sec*1000000 + (long long)all_sendrecv_dalay.tv_usec)/SET_SEND_PACKAGE_NUMBER);

}

#endif