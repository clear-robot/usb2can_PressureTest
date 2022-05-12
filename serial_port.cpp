//
// Created by kaylor on 2022/4/6.
//

#include "serial_port.h"
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include "thread"
#include <sys/ioctl.h>
#include "time_stamp.h"
#include <sys/time.h>

fream send_control_send_fream_list[SET_SEND_PACKAGE_NUMBER] = {0};
fream recvcontol_send_fream_list[SET_SEND_PACKAGE_NUMBER] = {0};

fream recv_fream_list[SET_SEND_PACKAGE_NUMBER] = {0};
fream recv_fream_list_sendThread[SET_SEND_PACKAGE_NUMBER] = {0};
fream recv_fream_list_chip1can1_sendto_chip2can1[SET_SEND_PACKAGE_NUMBER] = {0};

SerialPort::SerialPort() : stream_(), pthread_(NULL), RTS_flag_(TIOCM_RTS), DTR_flag_(TIOCM_DTR), running_flag_(true) {
}

SerialPort::~SerialPort() {
    running_flag_ = false;
    if (pthread_ != NULL) {
        pthread_->join();
        delete pthread_;
    }
    Close();
}

void PrintfBuf(char buf[], int len) {
    printf("len = %d\n", len);
    for (int i = 0; i < len; i++) {
        printf("%x  ", buf[i]);
    }
    printf("\n");
}

// char *itoa(int val, char *buf, unsigned radix) {
//     char *p;
//     char *firstdig;
//     char temp;
//     unsigned digval;
//     p = buf;
//     if (val < 0) {
//         *p++ = '-';
//         val = (unsigned long) (-(long) val);
//     }
//     firstdig = p;
//     do {
//         digval = (unsigned) (val % radix);
//         val /= radix;

//         if (digval > 9)
//             *p++ = (char) (digval - 10 + 'a');
//         else
//             *p++ = (char) (digval + '0');
//     } while (val > 0);

//     *p-- = '\0 ';
//     do {
//         temp = *p;
//         *p = *firstdig;
//         *firstdig = temp;
//         --p;
//         ++firstdig;
//     } while (firstdig < p);
//     return buf;
// }

// int CreateFileToSaveResult(FILE *pFile) {
//     std::string str = "";
//     char file_name[50] = "Receive_interval";
//     char temp_file[50] = {0};
//     itoa(SET_SEND_INTERVAL, temp_file, 10);
//     strcat(file_name, temp_file);
//     strcat(file_name, "us_Send");
//     itoa(SET_SEND_PACKAGE_NUMBER, temp_file, 10);
//     strcat(file_name, temp_file);
//     strcat(file_name, ".txt");

//     pFile = fopen(file_name, "w");
//     if (pFile == NULL) {
//         printf("Create/Open %s fail\n", file_name);
//         return 0;
//     } else {
//         return 1;
//     }

// }

// //Write the data and time to file
// int WriteResultToFile(FILE *pFile,char *buf,uint32_t receive_last_package_time){
//     for(int i = 0;i < 8;i++){
//         fprintf(pFile,"%x",buf[i+10]);
//     }
//     // fprintf (pFile, ",%ld\n",receive_last_package_time);
// }

uint32_t chip1can2_sendto_chip2can2_recvnumber = 1;

uint32_t receive_package_number_SendThread = 1;
uint32_t chip1_can1_sendto_chip2_can1_recvnumber = 1;
void *ReceiveThread(void *arg) {
    SerialPort *serial_port = (SerialPort *) (arg);
    uint32_t kReceive_overtime = 4 * 1000 * 1000;  // Receive overtime receive
    struct timeval receive_first_package_time;
    struct timeval receive_last_package_time;
    gettimeofday(&receive_last_package_time,0);
    
    struct timeval now_time;  //get now time
    struct timeval temp_time;  //temp time
    uint8_t buf[1024];
    int len = 0;

    while (chip1can2_sendto_chip2can2_recvnumber <= SET_SEND_PACKAGE_NUMBER || chip1_can1_sendto_chip2_can1_recvnumber <= SET_SEND_PACKAGE_NUMBER){
        len = read(serial_port->fd_, buf, sizeof(buf));
        if (len > 0) {
            gettimeofday(&receive_last_package_time,0);

            if (chip1can2_sendto_chip2can2_recvnumber == 1) {
                gettimeofday(&receive_first_package_time,0);
            }
            
            // printf("len = %d\n", len);
            // for (size_t i = 0; i < len; i++)
            // {
            //     printf("%02X ", buf[i]);
            // }
            // printf("\n");
            for (int i_len = 0;i_len < len/19;i_len++){
                #if 0
                if (buf[1 + i_len*19] == 0x02){
                    memcpy(recv_fream_list[chip1can2_sendto_chip2can2_recvnumber].canData, &buf[10], 8);
                    gettimeofday(&recv_fream_list[chip1can2_sendto_chip2can2_recvnumber].time,0);
                    chip1can2_sendto_chip2can2_recvnumber++;
                    // printf("receive_package_number_SendThread = %d\n",receive_package_number_SendThread);
                }
                #endif
                #if 1
                if (buf[8 + i_len*19] == 0x01){
                    memcpy(recv_fream_list[chip1can2_sendto_chip2can2_recvnumber].canData, &buf[10], 8);
                    gettimeofday(&recv_fream_list[chip1can2_sendto_chip2can2_recvnumber].time,0);

                    // printf("chip1can2_sendto_chip2can2_recvnumber = %d\n",chip1can2_sendto_chip2can2_recvnumber);

                    chip1can2_sendto_chip2can2_recvnumber++;

                }else if(buf[8 + i_len*19] == 0x00){  //chip1_can1_sendto_chip2_can1_listcmp
                    memcpy(recv_fream_list_chip1can1_sendto_chip2can1[chip1_can1_sendto_chip2_can1_recvnumber].canData, &buf[10], 8);
                    gettimeofday(&recv_fream_list_chip1can1_sendto_chip2can1[chip1_can1_sendto_chip2_can1_recvnumber].time,0);
                    
                    // printf("chip1_can1_sendto_chip2_can1_recvnumber = %d\n",chip1_can1_sendto_chip2_can1_recvnumber);
                    chip1_can1_sendto_chip2_can1_recvnumber++;

                }else{
                    printf("this is bug\n");
                }
                #endif
            }
            len = 0;
        }
        gettimeofday(&now_time,0);
        temp_time = TimeStamp::SubTime(now_time,receive_last_package_time);

        if ((temp_time.tv_sec*1000000 + temp_time.tv_usec) > kReceive_overtime) {
            printf("receive overtime 4s\n");
            break;
        }
    }

    if(chip1_can1_sendto_chip2_can1_recvnumber == SET_SEND_PACKAGE_NUMBER+1){
        chip1_can1_sendto_chip2_can1_recvnumber--;
     }
    if(chip1can2_sendto_chip2can2_recvnumber == (SET_SEND_PACKAGE_NUMBER+1)){
        chip1can2_sendto_chip2can2_recvnumber--;
    }
    struct timeval alltime = TimeStamp::SubTime(receive_last_package_time,receive_first_package_time);
    printf("Receive Package failure Number:%d\n",SET_SEND_PACKAGE_NUMBER - chip1can2_sendto_chip2can2_recvnumber);
    printf("all bytes:%d\n",19*chip1can2_sendto_chip2can2_recvnumber);
    long long recv_alltime = alltime.tv_sec*1000000 + alltime.tv_usec;

    printf("exit receive thread\n");
    return NULL;
}

//uint32_t receive_package_number_SendThread = 1;
void *ReceiveThread_Send(void *arg) {
    SerialPort *serial_port = (SerialPort *) (arg);
    uint32_t kReceive_overtime = 4 * 1000 * 1000;  // Receive overtime receive
    struct timeval receive_first_package_time;
    struct timeval receive_last_package_time;
    gettimeofday(&receive_last_package_time,0);
    
    struct timeval now_time;  //get now time
    struct timeval temp_time;  //temp time
    uint8_t buf[1024];
    int len = 0;
    while (receive_package_number_SendThread <= SET_SEND_PACKAGE_NUMBER) {
        len = read(serial_port->fd_, buf, sizeof(buf));
        if (len > 0){
            gettimeofday(&receive_last_package_time,0);

            if (receive_package_number_SendThread == 1) {
                gettimeofday(&receive_first_package_time,0);
            }
            // Write the data and time to file
            // Save the recv mask to recv_list
            // memcpy(recv_fream_list_sendThread[receive_package_number_SendThread].canData, &buf[10], 8);
            // gettimeofday(&recv_fream_list_sendThread[receive_package_number_SendThread].time,0);
                // printf("len = %d\n", len);
                // for (size_t i = 0; i < len; i++)
                // {
                //     printf("%02X ", buf[i]);
                // }
                // printf("\n");

            for (int i_len = 0;i_len < len/19;i_len++){
                // printf("buf[%d] = %x\t",1 + i_len*19,buf[1 + i_len*19]);
                if (buf[1 + i_len*19] == 0x02){
                    memcpy(recv_fream_list_sendThread[receive_package_number_SendThread].canData, &buf[10], 8);
                    gettimeofday(&recv_fream_list_sendThread[receive_package_number_SendThread].time,0);
                    receive_package_number_SendThread++;
                    // printf("receive_package_number_SendThread = %d\n",receive_package_number_SendThread);
                }
            }
            // if (buf[1] == 0x02){
                
            //     receive_package_number_SendThread++;
            //     printf("receive_package_number_SendThread = %d\n",receive_package_number_SendThread);
            // }
            
            len = 0;
        }
        gettimeofday(&now_time,0);
        temp_time = TimeStamp::SubTime(now_time,receive_last_package_time);

        if ((temp_time.tv_sec*1000000 + temp_time.tv_usec) > kReceive_overtime) {
            printf("receive overtime 8s\n");
            break;
        }
    }

    if(receive_package_number_SendThread == SET_SEND_PACKAGE_NUMBER+1){
        receive_package_number_SendThread--;
    }

    struct timeval alltime = TimeStamp::SubTime(receive_last_package_time,receive_first_package_time);
    printf("ReceiveThread_Send Receive Package failure Number:%d\n",SET_SEND_PACKAGE_NUMBER - receive_package_number_SendThread);
    printf("all bytes:%d\n",19*receive_package_number_SendThread);
    long long recv_alltime = alltime.tv_sec*1000000 + alltime.tv_usec;

    printf("exit receive thread\n");
    return NULL;
}

int SerialPort::open_send_port(const char *dev, int baud, int dataBits, int parityMode, int stopBits) {
    struct termios options;
    bzero(&options, sizeof(options));
    int baudT = TransformBaud(baud);
    if (baudT < 0)
        return BAUD_NOT_SUPPORTED;
    //  Configure the input baud rate
    cfsetispeed(&options, baudT);
    //  Configure the output baud rate
    cfsetospeed(&options, baudT);
    int dataBitsT = TransformDataBits(dataBits);
    if (dataBitsT < 0)
        return DATABITS_NOT_SUPPORTED;
    //  Set the data bits = dataBitsT
    options.c_cflag |= dataBitsT;
    //  Disable/Enable the Parity Enable bit(PARENB)
    if (parityMode == PARITY_ODD) {
        options.c_cflag |= PARENB;
        options.c_cflag |= PARODD;
    } else if (parityMode == PARITY_EVEN) {
        options.c_cflag |= PARENB;
    } else if (parityMode != PARITY_NONE) {
        return PARITYMODE_NOT_SUPPORTED;
    }
    if (stopBits == 2) {
        options.c_cflag |= CSTOPB;
    } else if (stopBits != 1) {
        return STOPBITS_NOT_SUPPORTED;
    }
    // Read at least 1 character
    options.c_cc[VMIN] = 1;
    // Set timeout = 1*100 ms from the 1st character is received
    options.c_cc[VTIME] = 1;

//  fd_ = ::open(dev, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    fd_ = ::open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd_ < 0)
        return DEV_NOT_FOUND;
    if (tcsetattr(fd_, TCSANOW, &options))
        return CONFIG_FAIL;
    /*------------------------- Putting MAX485 chip in USB2SERIAL in Receive Mode ----------------------------*/
    //                                                                                                        //
    //	----+			+-----------+              L  +-----------+                                           //
    //		|			| 	    ~RTS| --------------> |~RE        |                                           //
    //	 PC |<==========| FT232     |                 |   MAX485  +(A,B)<~~~~~~~~~~~~~~~Data in(RS485)        //
    //	    |    USB    |       ~DTR| --------------> | DE        |        Twisted Pair                       //
    //  ----+			+-----------+              L  +-----------+                                           //
    //                                                                                                        //
    //--------------------------------------------------------------------------------------------------------//
    // Set RX mode
    SetRxMode();
    SetTxMode();
    if (tcflush(fd_, TCIOFLUSH))
        return CONFIG_FAIL;
     pthread_ = new std::thread(ReceiveThread_Send, this);
    return OK;
}

int SerialPort::Open(const char *dev, int baud, int dataBits, int parityMode, int stopBits) {
    struct termios options;
    bzero(&options, sizeof(options));
    int baudT = TransformBaud(baud);
    if (baudT < 0)
        return BAUD_NOT_SUPPORTED;
    //  Configure the input baud rate
    cfsetispeed(&options, baudT);
    //  Configure the output baud rate
    cfsetospeed(&options, baudT);
    int dataBitsT = TransformDataBits(dataBits);
    if (dataBitsT < 0)
        return DATABITS_NOT_SUPPORTED;
    //  Set the data bits = dataBitsT
    options.c_cflag |= dataBitsT;
    //  Disable/Enable the Parity Enable bit(PARENB)
    if (parityMode == PARITY_ODD) {
        options.c_cflag |= PARENB;
        options.c_cflag |= PARODD;
    } else if (parityMode == PARITY_EVEN) {
        options.c_cflag |= PARENB;
    } else if (parityMode != PARITY_NONE) {
        return PARITYMODE_NOT_SUPPORTED;
    }
    if (stopBits == 2) {
        options.c_cflag |= CSTOPB;
    } else if (stopBits != 1) {
        return STOPBITS_NOT_SUPPORTED;
    }
    // Read at least 1 character
    options.c_cc[VMIN] = 1;
    // Set timeout = 1*100 ms from the 1st character is received
    options.c_cc[VTIME] = 1;

//  fd_ = ::open(dev, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    fd_ = ::open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd_ < 0)
        return DEV_NOT_FOUND;
    if (tcsetattr(fd_, TCSANOW, &options))
        return CONFIG_FAIL;
    /*------------------------- Putting MAX485 chip in USB2SERIAL in Receive Mode ----------------------------*/
    //                                                                                                        //
    //	----+			+-----------+              L  +-----------+                                           //
    //		|			| 	    ~RTS| --------------> |~RE        |                                           //
    //	 PC |<==========| FT232     |                 |   MAX485  +(A,B)<~~~~~~~~~~~~~~~Data in(RS485)        //
    //	    |    USB    |       ~DTR| --------------> | DE        |        Twisted Pair                       //
    //  ----+			+-----------+              L  +-----------+                                           //
    //                                                                                                        //
    //--------------------------------------------------------------------------------------------------------//
    // Set RX mode
    SetRxMode();
    if (tcflush(fd_, TCIOFLUSH))
        return CONFIG_FAIL;
    pthread_ = new std::thread(ReceiveThread, this);
    return OK;
}


int SerialPort::Close() {
    if (fd_ >= 0) {
        int status = close(fd_);
        fd_ = -1;
        return status;
    }
    return 0;
}

int SerialPort::Write(const char *data, int len) {
    SetTxMode();
    int ret = ::write(fd_, data, len);
    //printf("Write %lld\n",TimeStamp::Now());
    SetRxMode();
    return ret;
}

int SerialPort::Available() {
    int len = stream_.GetLength();
    return len;
}

int SerialPort::Peek(char *buf, int len) {
    len = stream_.Peek(buf, len);
    return len;
}

int SerialPort::Read(char *buf, int len, int timeout) {
    timestamp_t start = TimeStamp::Now();
    int total = 0;
    while (total < len) {
        mutex_.lock();
        int readLen = stream_.Take(buf + total, len - total);
        mutex_.unlock();
        if (readLen > 0)
            total += readLen;
        timestamp_t now = TimeStamp::Now();
        if (now >= start + timeout)
            break;
//    usleep(1000);
    }
    return total;
}

int SerialPort::Read(char *buf, int maxLen, const char *end, int timeout, int *recvLen) {
    int endLen = strlen(end);
    timestamp_t start = TimeStamp::Now();
    int total = 0;
    while (total < maxLen) {
        mutex_.lock();
        int readLen = stream_.Take(buf + total, 1);
        mutex_.unlock();
        if (readLen > 0) {
            total += readLen;
            if (EndsWith(buf, total, end, endLen)) {
                if (recvLen != 0)
                    *recvLen = total;
                return READ_END;
            }
        }
        timestamp_t now = TimeStamp::Now();
        if (now >= start + timeout)
            return READ_TIMEOUT;
//    usleep(1000);
    }
    return READ_BUFFER_FULL;
}

int SerialPort::TransformBaud(int baud) {
    int map[][2] = {{2400,    B2400},
                    {4800,    B4800},
                    {9600,    B9600},
                    {19200,   B19200},
                    {38400,   B38400},
                    {57600,   B57600},
                    {115200,  B115200},
                    {230400,  B230400},
                    {460800,  B460800},
                    {500000,  B500000},
                    {576000,  B576000},
                    {921600,  B921600},
                    {1000000, B1000000},
                    {1152000, B1152000},
                    {1500000, B1500000},
                    {2000000, B2000000},
                    {2500000, B2500000},
                    {3000000, B3000000},
                    {3500000, B3500000},
                    {4000000, B4000000}};
    for (int i = 0; i < sizeof(map) / sizeof(int) / 2; i++)
        if (map[i][0] == baud)
            return map[i][1];
    return -1;
}

int SerialPort::TransformDataBits(int dataBits) {
    int map[][2] = {{5, CS5},
                    {6, CS6},
                    {7, CS7},
                    {8, CS8}};
    for (int i = 0; i < sizeof(map) / sizeof(int) / 2; i++)
        if (map[i][0] == dataBits)
            return map[i][1];
    return -1;
}

bool SerialPort::EndsWith(const char *str, int strLen, const char *end, int endLen) {
    if (strLen < endLen)
        return false;
    for (int i = endLen - 1; i >= 0; i--)
        if (end[i] != str[strLen - endLen + i])
            return false;
    return true;
}

void SerialPort::SetRxMode(void) {
    ioctl(fd_, TIOCMBIS, &RTS_flag_);
    ioctl(fd_, TIOCMBIS, &DTR_flag_);
}

void SerialPort::SetTxMode(void) {
    ioctl(fd_, TIOCMBIC, &RTS_flag_);
    ioctl(fd_, TIOCMBIC, &DTR_flag_);
}