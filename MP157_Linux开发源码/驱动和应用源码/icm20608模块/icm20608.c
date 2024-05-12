/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名		: icm20608.c
作者	  	: 正点原子Linux团队
版本	   	: V1.0
描述	   	: ICM20608 SPI驱动程序
其他	   	: 无
论坛 	   	: www.openedv.com
日志	   	: 初版V1.0 2021/03/22 正点原子Linux团队创建
***************************************************************/
#include <linux/spi/spi.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include "icm20608reg.h"
#include <linux/gpio.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>

#define ICM20608_CNT	1
#define ICM20608_NAME	"icm20608"

struct icm20608_dev {
	struct spi_device *spi;		/* spi设备 */
	dev_t devid;				/* 设备号 	 */
	struct cdev cdev;			/* cdev 	*/
	struct class *class;		/* 类 		*/
	struct device *device;		/* 设备 	 */
	struct device_node	*nd; 	/* 设备节点 */
	signed int gyro_x_adc;		/* 陀螺仪X轴原始值 	 */
	signed int gyro_y_adc;		/* 陀螺仪Y轴原始值		*/
	signed int gyro_z_adc;		/* 陀螺仪Z轴原始值 		*/
	signed int accel_x_adc;		/* 加速度计X轴原始值 	*/
	signed int accel_y_adc;		/* 加速度计Y轴原始值	*/
	signed int accel_z_adc;		/* 加速度计Z轴原始值 	*/
	signed int temp_adc;		/* 温度原始值 			*/
};

/*
 * @description	: 从icm20608读取多个寄存器数据
 * @param - dev:  icm20608设备
 * @param - reg:  要读取的寄存器首地址
 * @param - val:  读取到的数据
 * @param - len:  要读取的数据长度
 * @return 		: 操作结果
 */
static int icm20608_read_regs(struct icm20608_dev *dev, u8 reg, void *buf, int len)
{

	int ret = -1;
	unsigned char txdata[1];
	unsigned char * rxdata;
	struct spi_message m;
	struct spi_transfer *t;
	struct spi_device *spi = (struct spi_device *)dev->spi;
    
	t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);	/* 申请内存 */
	if(!t) {
		return -ENOMEM;
	}
	rxdata = kzalloc(sizeof(char) * len, GFP_KERNEL);	/* 申请内存 */
	if(!rxdata) {
		goto out1;
	}
	/* 一共发送len+1个字节的数据，第一个字节为
	寄存器首地址，一共要读取len个字节长度的数据，*/
	txdata[0] = reg | 0x80;		/* 写数据的时候首寄存器地址bit8要置1 */			
	t->tx_buf = txdata;			/* 要发送的数据 */
    t->rx_buf = rxdata;			/* 要读取的数据 */
	t->len = len+1;				/* t->len=发送的长度+读取的长度 */
	spi_message_init(&m);		/* 初始化spi_message */
	spi_message_add_tail(t, &m);/* 将spi_transfer添加到spi_message队列 */
	ret = spi_sync(spi, &m);	/* 同步发送 */
	if(ret) {
		goto out2;
	}
	
    memcpy(buf , rxdata+1, len);  /* 只需要读取的数据 */

out2:
	kfree(rxdata);					/* 释放内存 */
out1:	
	kfree(t);						/* 释放内存 */
	
	return ret;
}

/*
 * @description	: 向icm20608多个寄存器写入数据
 * @param - dev:  icm20608设备
 * @param - reg:  要写入的寄存器首地址
 * @param - val:  要写入的数据缓冲区
 * @param - len:  要写入的数据长度
 * @return 	  :   操作结果
 */
static s32 icm20608_write_regs(struct icm20608_dev *dev, u8 reg, u8 *buf, u8 len)
{
	int ret = -1;
	unsigned char *txdata;
	struct spi_message m;
	struct spi_transfer *t;
	struct spi_device *spi = (struct spi_device *)dev->spi;
	
	t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);	/* 申请内存 */
	if(!t) {
		return -ENOMEM;
	}
	
	txdata = kzalloc(sizeof(char)+len, GFP_KERNEL);
	if(!txdata) {
		goto out1;
	}
	
	/* 一共发送len+1个字节的数据，第一个字节为
	寄存器首地址，len为要写入的寄存器的集合，*/
	*txdata = reg & ~0x80;	/* 写数据的时候首寄存器地址bit8要清零 */
    memcpy(txdata+1, buf, len);	/* 把len个寄存器拷贝到txdata里，等待发送 */
	t->tx_buf = txdata;			/* 要发送的数据 */
	t->len = len+1;				/* t->len=发送的长度+读取的长度 */
	spi_message_init(&m);		/* 初始化spi_message */
	spi_message_add_tail(t, &m);/* 将spi_transfer添加到spi_message队列 */
	ret = spi_sync(spi, &m);	/* 同步发送 */
    if(ret) {
        goto out2;
    }
	
out2:
	kfree(txdata);				/* 释放内存 */
out1:
	kfree(t);					/* 释放内存 */
	return ret;
}

/*
 * @description	: 读取icm20608指定寄存器值，读取一个寄存器
 * @param - dev:  icm20608设备
 * @param - reg:  要读取的寄存器
 * @return 	  :   读取到的寄存器值
 */
static unsigned char icm20608_read_onereg(struct icm20608_dev *dev, u8 reg)
{
	u8 data = 0;
	icm20608_read_regs(dev, reg, &data, 1);
	return data;
}

/*
 * @description	: 向icm20608指定寄存器写入指定的值，写一个寄存器
 * @param - dev:  icm20608设备
 * @param - reg:  要写的寄存器
 * @param - data: 要写入的值
 * @return   :    无
 */	

static void icm20608_write_onereg(struct icm20608_dev *dev, u8 reg, u8 value)
{
	u8 buf = value;
	icm20608_write_regs(dev, reg, &buf, 1);
}

/*
 * @description	: 读取ICM20608的数据，读取原始数据，包括三轴陀螺仪、
 * 				: 三轴加速度计和内部温度。
 * @param - dev	: ICM20608设备
 * @return 		: 无。
 */
void icm20608_readdata(struct icm20608_dev *dev)
{
	unsigned char data[14];
	icm20608_read_regs(dev, ICM20_ACCEL_XOUT_H, data, 14);

	dev->accel_x_adc = (signed short)((data[0] << 8) | data[1]); 
	dev->accel_y_adc = (signed short)((data[2] << 8) | data[3]); 
	dev->accel_z_adc = (signed short)((data[4] << 8) | data[5]); 
	dev->temp_adc    = (signed short)((data[6] << 8) | data[7]); 
	dev->gyro_x_adc  = (signed short)((data[8] << 8) | data[9]); 
	dev->gyro_y_adc  = (signed short)((data[10] << 8) | data[11]);
	dev->gyro_z_adc  = (signed short)((data[12] << 8) | data[13]);
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做pr似有ate_data的成员变量
 * 					  一般在open的时候将private_data似有向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int icm20608_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * @description		: 从设备读取数据 
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t icm20608_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	signed int data[7];
	long err = 0;
	struct cdev *cdev = filp->f_path.dentry->d_inode->i_cdev;
	struct icm20608_dev *dev = container_of(cdev, struct icm20608_dev, cdev);
            
	icm20608_readdata(dev);
	data[0] = dev->gyro_x_adc;
	data[1] = dev->gyro_y_adc;
	data[2] = dev->gyro_z_adc;
	data[3] = dev->accel_x_adc;
	data[4] = dev->accel_y_adc;
	data[5] = dev->accel_z_adc;
	data[6] = dev->temp_adc;
	err = copy_to_user(buf, data, sizeof(data));
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int icm20608_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* icm20608操作函数 */
static const struct file_operations icm20608_ops = {
	.owner = THIS_MODULE,
	.open = icm20608_open,
	.read = icm20608_read,
	.release = icm20608_release,
};

/*
 * ICM20608内部寄存器初始化函数 
 * @param - spi : 要操作的设备
 * @return 	: 无
 */
void icm20608_reginit(struct icm20608_dev *dev)
{
	u8 value = 0;
	
	icm20608_write_onereg(dev, ICM20_PWR_MGMT_1, 0x80);
	mdelay(50);
	icm20608_write_onereg(dev, ICM20_PWR_MGMT_1, 0x01);
	mdelay(50);

	value = icm20608_read_onereg(dev, ICM20_WHO_AM_I);
	printk("ICM20608 ID = %#X\r\n", value);	

	icm20608_write_onereg(dev, ICM20_SMPLRT_DIV, 0x00); 	/* 输出速率是内部采样率					*/
	icm20608_write_onereg(dev, ICM20_GYRO_CONFIG, 0x18); 	/* 陀螺仪±2000dps量程 				*/
	icm20608_write_onereg(dev, ICM20_ACCEL_CONFIG, 0x18); 	/* 加速度计±16G量程 					*/
	icm20608_write_onereg(dev, ICM20_CONFIG, 0x04); 		/* 陀螺仪低通滤波BW=20Hz 				*/
	icm20608_write_onereg(dev, ICM20_ACCEL_CONFIG2, 0x04); /* 加速度计低通滤波BW=21.2Hz 			*/
	icm20608_write_onereg(dev, ICM20_PWR_MGMT_2, 0x00); 	/* 打开加速度计和陀螺仪所有轴 				*/
	icm20608_write_onereg(dev, ICM20_LP_MODE_CFG, 0x00); 	/* 关闭低功耗 						*/
	icm20608_write_onereg(dev, ICM20_FIFO_EN, 0x00);		/* 关闭FIFO						*/
}

/*
  * @description     : spi驱动的probe函数，当驱动与
  *                    设备匹配以后此函数就会执行
  * @param - spi  	: spi设备
  * 
  */	
static int icm20608_probe(struct spi_device *spi)
{
	int ret;
	struct icm20608_dev *icm20608dev;
	
	/* 分配icm20608dev对象的空间 */
	icm20608dev = devm_kzalloc(&spi->dev, sizeof(*icm20608dev), GFP_KERNEL);
	if(!icm20608dev)
		return -ENOMEM;
		
	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	ret = alloc_chrdev_region(&icm20608dev->devid, 0, ICM20608_CNT, ICM20608_NAME);
	if(ret < 0) {
		pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n", ICM20608_NAME, ret);
        return 0;
	}

	/* 2、初始化cdev */
	icm20608dev->cdev.owner = THIS_MODULE;
	cdev_init(&icm20608dev->cdev, &icm20608_ops);
	
	/* 3、添加一个cdev */
	ret = cdev_add(&icm20608dev->cdev, icm20608dev->devid, ICM20608_CNT);
	if(ret < 0) {
		goto del_unregister;
	}
	
	/* 4、创建类 */
	icm20608dev->class = class_create(THIS_MODULE, ICM20608_NAME);
	if (IS_ERR(icm20608dev->class)) {
		goto del_cdev;
	}

	/* 5、创建设备 */
	icm20608dev->device = device_create(icm20608dev->class, NULL, icm20608dev->devid, NULL, ICM20608_NAME);
	if (IS_ERR(icm20608dev->device)) {
		goto destroy_class;
	}
	icm20608dev->spi = spi;
	
	/*初始化spi_device */
	spi->mode = SPI_MODE_0;	/*MODE0，CPOL=0，CPHA=0*/
	spi_setup(spi);
	
	/* 初始化ICM20608内部寄存器 */
	icm20608_reginit(icm20608dev);	
	/* 保存icm20608dev结构体 */
	spi_set_drvdata(spi, icm20608dev);

	return 0;
destroy_class:
	device_destroy(icm20608dev->class, icm20608dev->devid);
del_cdev:
	cdev_del(&icm20608dev->cdev);
del_unregister:
	unregister_chrdev_region(icm20608dev->devid, ICM20608_CNT);
	return -EIO;
}

/*
 * @description     : spi驱动的remove函数，移除spi驱动的时候此函数会执行
 * @param - spi 	: spi设备
 * @return          : 0，成功;其他负值,失败
 */
static int icm20608_remove(struct spi_device *spi)
{
	struct icm20608_dev *icm20608dev = spi_get_drvdata(spi);
	/* 注销字符设备驱动 */
	/* 1、删除cdev */
	cdev_del(&icm20608dev->cdev);
	/* 2、注销设备号 */
	unregister_chrdev_region(icm20608dev->devid, ICM20608_CNT); 
	/* 3、注销设备 */
	device_destroy(icm20608dev->class, icm20608dev->devid);
	/* 4、注销类 */
	class_destroy(icm20608dev->class); 
	return 0;
}

/* 传统匹配方式ID列表 */
static const struct spi_device_id icm20608_id[] = {
	{"alientek,icm20608", 0},
	{}
};

/* 设备树匹配列表 */
static const struct of_device_id icm20608_of_match[] = {
	{ .compatible = "alientek,icm20608" },
	{ /* Sentinel */ }
};

/* SPI驱动结构体 */
static struct spi_driver icm20608_driver = {
	.probe = icm20608_probe,
	.remove = icm20608_remove,
	.driver = {
			.owner = THIS_MODULE,
		   	.name = "icm20608",
		   	.of_match_table = icm20608_of_match,
		   },
	.id_table = icm20608_id,
};

/*
 * @description	: 驱动入口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init icm20608_init(void)
{
	return spi_register_driver(&icm20608_driver);
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit icm20608_exit(void)
{
	spi_unregister_driver(&icm20608_driver);
}

module_init(icm20608_init);
module_exit(icm20608_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALIENTEK");
MODULE_INFO(intree, "Y");
