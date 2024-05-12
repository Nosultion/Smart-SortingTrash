#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SERVER_IP "112.125.89.8"
#define SERVER_PORT 46032

int set_uart(int fd,int nSpeed, int nBits, char nEvent, int nStop);

int main(int argc, char *argv[]) {
    int fd,fd2,fd3;
    char *filename;
    signed int databuf[7];
    unsigned short databuf2[3];
    unsigned char data[14];
    signed int gyro_x_adc, gyro_y_adc, gyro_z_adc;
    signed int accel_x_adc, accel_y_adc, accel_z_adc;
    signed int temp_adc;
    float gyro_x_act, gyro_y_act, gyro_z_act;
    float accel_x_act, accel_y_act, accel_z_act;
    float temp_act;
    int ret = 0;
    int ret2 = 0;
    int ret3 = 0;
    unsigned short ir, als, ps;
    char buf[1024] = "Embedfire tty send test.\n";

    if (argc != 4) {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];
    fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("can't open file %s\r\n", filename);
        return -1;
    }
    filename = argv[2];
    fd2 = open(filename, O_RDWR);
    if (fd2 < 0) {
        printf("can't open file %s\r\n", filename);
        return -1;
    }

    filename = argv[3];
    fd3 = open(filename, O_RDWR|O_NONBLOCK);
    if (fd3 < 0) {
        printf("can't open file %s\r\n", filename);
        return -1;
    }
    if( set_uart(fd3,115200,8,'n',1) )
    {
        printf("set uart error\n");
    }

    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("invalid address or address not supported");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        return -1;
    }

    while (1) {
        ret = read(fd, databuf, sizeof(databuf));
        ret2 = read(fd2, databuf2, sizeof(databuf2));
        write(fd3, buf, strlen(buf));
        //printf("send res = %d bytes data: %s",strlen(buf), buf);
        memset(buf,0,1024);
        ret3 = read(fd3, buf, 1024);
        if(ret3 >= 0)
        {
            printf("Receive res = %d bytes data: %s\n",ret3, buf);
        }
        if (ret == 0&&ret2==0) {
            gyro_x_adc = databuf[0];
            gyro_y_adc = databuf[1];
            gyro_z_adc = databuf[2];
            accel_x_adc = databuf[3];
            accel_y_adc = databuf[4];
            accel_z_adc = databuf[5];
            temp_adc = databuf[6];

            gyro_x_act = (float)(gyro_x_adc)  / 16.4;
            gyro_y_act = (float)(gyro_y_adc)  / 16.4;
            gyro_z_act = (float)(gyro_z_adc)  / 16.4;
            accel_x_act = (float)(accel_x_adc) / 2048;
            accel_y_act = (float)(accel_y_adc) / 2048;
            accel_z_act = (float)(accel_z_adc) / 2048;
            temp_act = ((float)(temp_adc) - 25 ) / 326.8 + 25;

			ir = databuf2[0]; /* 红外线传感器数据 */
			als = databuf2[1]; /* 环境光传感器数据 */
			ps = databuf2[2];  /* 接近传感器数据 */

            // 构建要发送的消息
            char message[256];
            // sprintf(message, "{X轴=%.2f, gyro_y_act=%.2f, gyro_z_act=%.2f, "
            //                  "accel_x_act=%.2f, accel_y_act=%.2f, accel_z_act=%.2f, "
            //                  "temp_act=%.2f, ir=%d,als=%d, ps=%d, 垃圾类别:%d",
            //                  gyro_x_act, gyro_y_act, gyro_z_act,
            //                  accel_x_act, accel_y_act, accel_z_act,
            //                  temp_act,ir,als,ps,buf[0]);
            sprintf(message, "{\"alsData\":\"%d\",\"psData\":\"%d\",\"irData\":\"%d\","
                             "\"gyro_x_act\":\"%.2f\",\"gyro_y_act\":\"%.2f\",\"gyro_z_act\":\"%.2f\","
                             "\"accel_x_act\":\"%.2f\",\"accel_y_act\":\"%.2f\",\"accel_z_act\":\"%.2f\","
                             "\"temp_act\":\"%.2f\",\"Trash_Type\":\"%d\"}",
                             als, ps, ir,
                             gyro_x_act, gyro_y_act, gyro_z_act,
                             accel_x_act, accel_y_act, accel_z_act,
                             temp_act, buf[0]);
            // 发送消息
            send(sockfd, message, strlen(message), 0);
        }

        usleep(1000000); // 100ms
    }

    close(fd);
    close(fd2);
    close(fd3);
    close(sockfd);

    return 0;
}

int set_uart(int fd,int nSpeed, int nBits, char nEvent, int nStop)
 {

     struct termios opt;

     //清空串口接收缓冲区
     tcflush(fd, TCIOFLUSH);

     //获取串口配置参数
     tcgetattr(fd, &opt);

     opt.c_cflag &= (~CBAUD);    //清除数据位设置
     opt.c_cflag &= (~PARENB);   //清除校验位设置

     //opt.c_iflag |= IGNCR;       //忽略接收数据中的'\r'字符，在windows中换行为'\r\n'
     opt.c_iflag &= (~ICRNL);    //不将'\r'转换为'\n'

     opt.c_lflag &= (~ECHO);     //不使用回显

     //设置波特率
     switch(nSpeed)
     {
         case 2400:
             cfsetspeed(&opt,B2400);
             break;

         case 4800:
             cfsetspeed(&opt,B4800);
             break;

         case 9600:
             cfsetspeed(&opt,B9600);
             break;

         case 38400:
             cfsetspeed(&opt,B38400);
             break;

         case 115200:
             cfsetspeed(&opt,B115200);
             break;

         default:
             return -1;
     }

     //设置数据位
     switch(nBits)
     {
         case 7:
             opt.c_cflag |= CS7;
             break;

         case 8:
             opt.c_cflag |= CS8;
             break;

         default:
             return -1;
     }

     //设置校验位
     switch(nEvent)
     {
         //无奇偶校验
         case 'n':
         case 'N':
             opt.c_cflag &= (~PARENB);
             break;

         //奇校验
         case 'o':
         case 'O':
             opt.c_cflag |= PARODD;
             break;

         //偶校验
         case 'e':
         case 'E':
             opt.c_cflag |= PARENB;
             opt.c_cflag &= (~PARODD);
             break;

         default:
             return -1;
     }

     //设置停止位
     switch(nStop)
     {
         case 1:
             opt.c_cflag &= ~CSTOPB;
             break;
         case 2:
             opt.c_cflag |= CSTOPB;
             break;
         default:
             return -1;
     }
     //设置串口
     tcsetattr(fd,TCSANOW,&opt);

     return 0;
 }
