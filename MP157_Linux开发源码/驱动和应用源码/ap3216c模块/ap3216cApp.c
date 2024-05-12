#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>

/*
 * @description    : 主程序
 * @param - argc   : argv数组元素个数
 * @param - argv   : 具体参数
 * @return         : 0 成功; 其他 失败
 */
int main(int argc, char *argv[])
{
    int fd;
    char *filename;
    unsigned short databuf[3];
    unsigned short ir, als, ps;
    int ret = 0;

    if (argc != 2)
    {
        printf("错误的用法！\r\n");
        return -1;
    }

    filename = argv[1];
    fd = open(filename, O_RDWR);

    if (fd < 0)
    {
        printf("无法打开文件 %s\r\n", filename);
        return -1;
    }

    int led_fd = open("/sys/class/leds/sys-led/brightness", O_RDWR);
    if (led_fd < 0)
    {
        printf("无法打开LED文件\r\n");
        close(fd);
        return -1;
    }

    while (1)
    {
        ret = read(fd, databuf, sizeof(databuf));

        if (ret == 0)
		{
			ir = databuf[0]; /* 红外线传感器数据 */
			als = databuf[1]; /* 环境光传感器数据 */
			ps = databuf[2];  /* 接近传感器数据 */

			// 计算als在0到5000范围内的百分比
			float als_percentage = (float)als / 5000.0;

			// 根据条件设置brightness_value
			int brightness_value;

			if (als <= 5000)
			{
				brightness_value = (int)(255.0 - als_percentage * 255.0);
			}
			else
			{
				brightness_value = 0;
			}

			// 确保亮度值在合理范围内
			if (brightness_value > 255)
				brightness_value = 255;
			else if (brightness_value < 0)
				brightness_value = 0;

			char brightness_str[10];
			snprintf(brightness_str, sizeof(brightness_str), "%d", brightness_value);
			write(led_fd, brightness_str, strlen(brightness_str));

			printf("红外线强度ir = %d 环境光强度als = %d 接近距离ps = %d LED亮度 = %d\n", ir, als, ps, (int)brightness_value);
		}
        usleep(200000); /* 100ms */
    }

    close(fd);    /* 关闭文件 */
    close(led_fd); /* 关闭LED文件 */

    return 0;
}
