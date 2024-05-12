#include "bsp_ps2.h"
#include "gpio.h"
#include <stdlib.h>

uint16_t Handkey;
uint8_t Comd[2]={0x01,0x42};	//开始命令。请求数据
uint8_t scan[9]={0x01,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//{0x01,0x42,0x00,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A};	// 类型读取

uint8_t Data[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //数据存储数组
uint16_t MASK[]={
    PSB_SELECT,
    PSB_L3,
    PSB_R3 ,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1 ,
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK
};	//按键值与按键名

void delay_1us(uint32_t us)
{
    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 1800000 * us);
    while (delay--)
	{
		;
	}
}

void PS2_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStructure.Pin = PS2_MOSI_PIN ;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;      /*设置引脚模式为通用推挽输出*/
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW; 
	HAL_GPIO_Init(PS2_MOSI_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = PS2_MISO_PIN ;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;      /*设置引脚模式为通用推挽输出*/
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW; 
	HAL_GPIO_Init(PS2_MISO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = PS2_CS_PIN ;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;      /*设置引脚模式为通用推挽输出*/
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW; 
	HAL_GPIO_Init(PS2_CS_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = PS2_SCK_PIN ;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;      /*设置引脚模式为通用推挽输出*/
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW; 
	HAL_GPIO_Init(PS2_SCK_PORT, &GPIO_InitStructure);
	DO_H;
	CLC_H;
	CS_H;

}

//读取手柄数据
uint8_t PS2_ReadData(uint8_t command)
{

	uint8_t i,j=1;
	uint8_t res=0; 
    for(i=0; i<=7; i++)          
    {
		if(command&0x01)
			DO_H;
		else
			DO_L;
		command = command >> 1;
		delay_1us(5);//10
		CLC_L;
		delay_1us(5); //10
		if(DI) 
			res = res + j;
		j = j << 1; 
		CLC_H;
		delay_1us(10);//10	 
    }
    DO_H;
	delay_1us(30);//50
    return res;	
}

//对读出来的 PS2 的数据进行处理
//按下为 0， 未按下为 1
unsigned char PS2_DataKey(void)
{
	uint8_t index = 0, i = 0;

	PS2_ClearData();
	CS_L;
	for(i=0;i<9;i++)	//更新扫描按键
	{
		Data[i] = PS2_ReadData(scan[i]);	
	} 
	CS_H;
	

	Handkey=(Data[4]<<8)|Data[3];     //这是16个按键  按下为0， 未按下为1
	for(index=0;index<16;index++)
	{	    
		if((Handkey&(1<<(MASK[index]-1)))==0)
			return index+1;
	}
	return 0;          //没有任何按键按下
}

//得到一个摇杆的模拟量	 范围0~256
uint8_t PS2_AnologData(uint8_t button)
{
	return Data[button];
}

//清除数据缓冲区
void PS2_ClearData(void)
{
	uint8_t a;
	for(a=0;a<9;a++)
		Data[a]=0x00;
}


/**
* Function       app_ps2_deal
* @author        liusen
* @date          2017.08.23    
* @brief         PS2协议处理
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
int key_sta=0,key_cnt=0,key_cnt_L1=0,key_cnt_L2=0,key_cnt_R1=0,key_cnt_R2=0;
int key_cnt_add=0,key_cnt_del=0;
void app_ps2_deal(void)
{
	uint8_t PS2_KEY = 0, X1=0,Y1=0,X2=0,Y2=0;

	PS2_KEY = PS2_DataKey();	 //手柄按键捕获处理
	   
	switch(PS2_KEY)
	{
		case PSB_SELECT:	 	printf("PSB_SELECT \n");  break;
		case PSB_L3:     		printf("PSB_L3 \n");  break;  
		case PSB_R3:				printf("PSB_R3 \n");  break;  
		case PSB_START:  		printf("PSB_START \n");  break;  
		case PSB_PAD_UP: 		
		{
			key_cnt_add++;
			if(key_cnt_add>=10)
			{
				key_cnt_add=0;
				if(key_cnt_L1>=0)key_cnt_L1++;
				else key_cnt_L1--;
				if(key_cnt_R1>=0)key_cnt_R1++;
				else key_cnt_R1--;
				if(key_cnt_L2>=0)key_cnt_L2++;
				else key_cnt_L2--;
				if(key_cnt_R2>=0)key_cnt_R2++;
				else key_cnt_R2--;
			}
			printf("PSB_PAD_UP \n");  
			break;
		}  
		case PSB_PAD_RIGHT:	
		{
			key_cnt_L1=abs(key_cnt_L1);
			key_cnt_R1=abs(key_cnt_R1);
			key_cnt_L2=abs(key_cnt_L2);
			key_cnt_R2=abs(key_cnt_R2);

			printf("PSB_PAD_RIGHT \n");  
			break;
		}
		case PSB_PAD_DOWN:	
		{
			key_cnt_del++;
			if(key_cnt_del>=10)
			{
				key_cnt_del=0;		
				if(key_cnt_L1>=0)key_cnt_L1--;
				else key_cnt_L1++;
				if(key_cnt_R1>=0)key_cnt_R1--;
				else key_cnt_R1++;
				if(key_cnt_L2>=0)key_cnt_L2--;
				else key_cnt_L2++;
				if(key_cnt_R2>=0)key_cnt_R2--;
				else key_cnt_R2++;
			}
			printf("PSB_PAD_DOWN \n"); 
			break;
		}
		case PSB_PAD_LEFT:	
		{
			key_cnt_L1=-1*abs(key_cnt_L1);
			key_cnt_R1=-1*abs(key_cnt_R1);
			key_cnt_L2=-1*abs(key_cnt_L2);
			key_cnt_R2=-1*abs(key_cnt_R2);
			printf("PSB_PAD_LEFT \n");  
			break; 
		}
		case PSB_L2:      	
		{
			printf("PSB_L2 \n");  	

		}break; 
		case PSB_R2:      	
		{

			printf("PSB_R2 \n");

		}  break; 
		case PSB_L1:      	
		{
			key_cnt_L1=key_cnt_R1=key_cnt_L2=key_cnt_R2=0;
			printf("PSB_L1 \n");  
			break;
		} 
		case PSB_R1:      	
		{
			key_cnt_L1=key_cnt_R1=key_cnt_L2=key_cnt_R2=0;
			printf("PSB_R1 \n");  
			break;  
		}   
		case PSB_TRIANGLE:	//前进
		{
			key_cnt_L1=-1*abs(key_cnt_L1);
			key_cnt_R1=-1*abs(key_cnt_R1);
			key_cnt_L2=abs(key_cnt_L2);
			key_cnt_R2=abs(key_cnt_R2);
			
			printf("PSB_TRIANGLE \n"); 
			break; 							
		}
		case PSB_CIRCLE:  	  	//右走
		{
			key_cnt_L1=abs(key_cnt_L1);
			key_cnt_R1=-1*abs(key_cnt_R1);
			key_cnt_L2=abs(key_cnt_L2);
			key_cnt_R2=-1*abs(key_cnt_R2);
			printf("PSB_CIRCLE \n"); 
			break;
		}
		case PSB_CROSS:   	//后退
		{
			key_cnt_L1=abs(key_cnt_L1);
			key_cnt_R1=abs(key_cnt_R1);
			key_cnt_L2=-1*abs(key_cnt_L2);
			key_cnt_R2=-1*abs(key_cnt_R2);
			printf("PSB_CROSS \n");  break; 					
		}
		case PSB_SQUARE:  //左走
		{
				key_cnt_L1=-1*abs(key_cnt_L1);
				key_cnt_R1=abs(key_cnt_R1);
				key_cnt_L2=-1*abs(key_cnt_L2);
				key_cnt_R2=abs(key_cnt_R2);
				printf("PSB_SQUARE \n"); 
				break;  	
		}			
		default: 					break; 
	}


	      

	//获取模拟值
	if(PS2_KEY == PSB_L1 || PS2_KEY == PSB_R1)
	{
		X1 = PS2_AnologData(PSS_LX);
		Y1 = PS2_AnologData(PSS_LY);
		X2 = PS2_AnologData(PSS_RX);
		Y2 = PS2_AnologData(PSS_RY);
		/*左摇杆*/
	    if (Y1 < 5 && X1 > 80 && X1 < 180) //上
	    {
	      
		  
	    }
	    else if (Y1 > 230 && X1 > 80 && X1 < 180) //下
	    {
	      
		  
	    }
	    else if (X1 < 5 && Y1 > 80 && Y1 < 180) //左
	    {
	      

	    }
	    else if (Y1 > 80 && Y1 < 180 && X1 > 230)//右
	    {
	     

	    }
	    else if (Y1 <= 80 && X1 <= 80) //左上
	    {
	  
	    }
	    else if (Y1 <= 80 && X1 >= 180) //右上
	    {
	      
	
	    }
	    else if (X1 <= 80 && Y1 >= 180) // 左下
	    {
	     
	    }
	    else if (Y1 >= 180 && X1 >= 180) //右下
	    {
	    
		  
	    }
	    else//停
	    {
	    
	    }

		 /*右摇杆*/
	    if (X2 < 5 && Y2 > 110 && Y2 < 150) //左
	    {
	     
	    }
	    else if (Y2 > 110 && Y2 < 150 && X2 > 230)//右
	    {
	    
	    }
	    else//归位
	    {
	     
	    }
	}

	
}

//void app_CarstateOutput(void)
//{
////根据小车状态做相应的动作
//	switch (g_CarState)
//	{
//		case enSTOP: Car_Stop(); break;
//		case enRUN: Car_Run(CarSpeedControl); break;
//		case enLEFT: Car_Left(CarSpeedControl); break;
//		case enRIGHT: Car_Right(CarSpeedControl); break;
//		case enBACK: Car_Back(CarSpeedControl); break;
//		case enTLEFT: Car_SpinLeft(CarSpeedControl, CarSpeedControl); break;
//		case enTRIGHT: Car_SpinRight(CarSpeedControl, CarSpeedControl); break;

//		case enUPLEFT:break;//左上转
//		case enDOWNLEFT:break;//左下转
//		case enUPRIGHT:break;//右上转 
//		case enDOWNRIGHT:break;//右下转  
//		default: Car_Stop(); break;
//	}

