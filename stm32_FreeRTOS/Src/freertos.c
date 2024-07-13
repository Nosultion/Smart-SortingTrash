/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  *
  * COPYRIGHT(c) 2024 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include "pid.h"
#include "bsp_can.h"
#include "mytype.h"
#include "tim.h"
#include "usart.h"
#include <stdlib.h>
#include "bsp_ps2.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId Task01Handle;
osThreadId Task02Handle;
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartTask01(void const * argument);
void StartTask02(void const * argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */
void duoji_smooth(int targetPosition_D,int targetPosition_L);
/* USER CODE BEGIN FunctionPrototypes */
int set_v,set_spd[4]={0,0,0,0};
uint8_t len,len2;
static int start_time=0;
int stop_time=500;
int last_time;
int trash_type=6;
int dianji_flag=0;//1代表有电机版本 0代表无电机版本（展示使用）

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(MyTask01, StartTask01, osPriorityNormal, 0, 128);
  Task01Handle = osThreadCreate(osThread(MyTask01), NULL);

  osThreadDef(MyTask02, StartTask02, osPriorityNormal, 0, 128);
  Task02Handle = osThreadCreate(osThread(MyTask02), NULL);
  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartTask01(void const * argument)
{

	my_can_filter_init_recv_all(&hcan1);
	HAL_CAN_Receive_IT(&hcan1, CAN_FIFO0);
//	HAL_CAN_Receive_IT(&hcan2, CAN_FIFO0);
//	PID_struct_init(&pid_omg, POSITION_PID, 20000, 20000,
//									1.5f,	0.1f,	0.0f	);  //angular rate closeloop.
	for(int i=0; i<4; i++)
	{
		PID_struct_init(&pid_spd[i], POSITION_PID, 20000, 20000,
									1.5f,	0.1f,	0.0f	);  //4 motos angular rate closeloop.
	}

	HAL_Delay(100);
  /* Infinite loop */
  for(;;)
  {
  
			for(int i=0; i<4; i++)
			{
				pid_calc(&pid_spd[i], moto_chassis[i].speed_rpm, set_spd[i]);
			}
			set_moto_current(&hcan1, 	pid_spd[0].pos_out, 
																pid_spd[1].pos_out,
																pid_spd[2].pos_out,
																pid_spd[3].pos_out);
		  
//			__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_1, 1000+set_spd[0]);//spd range[0,999]
//			__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_2, 1000+set_spd[1]);
//			__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, 1000+set_spd[2]);
//			__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_4, 1000+set_spd[3]);

	switch(key_sta)
	{
		case 0:	//no key
			if( 0 == HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) )
			{
				key_sta = 1;
			}
			break;
		case 1: //key down wait release.
			if( 0 == HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) )
			{
				key_sta = 2;
				key_cnt++;
			}
			else
			{
				key_sta = 0;
			}
			break;
		case 2: 
			if( 0 != HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) )
			{
				key_sta = 0;
			}
			break;
	}
	if(key_cnt>10)
		key_cnt = 0;

	set_spd[0] = key_cnt_L1*500;
	set_spd[1] = key_cnt_L2*500;
	set_spd[2] = key_cnt_R1*500;
	set_spd[3] = key_cnt_R2*500;
		
	HAL_Delay(20);
	//遥杆数据
	app_ps2_deal();
	
	osDelay(10);
  }
  /* USER CODE END StartDefaultTask */
}

void StartTask02(void const * argument)
{
	HAL_GPIO_WritePin(GPIOE, LED2_Pin, GPIO_PIN_SET);
	
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
	
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
	
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);
	
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
	
	

	
	HAL_Delay(100);
	
//电机控制	
//	__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_1, 1000);
//	__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_2, 1000);
//	__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, 1000);
//	__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_4, 1000);
	
//	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 25);
//	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, 25);	
//	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, 25);
//	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, 25);
	
//	__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_1, 100);
//	__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_2, 100);
//	__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, 100);
//	__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4, 100);
	
	//舵机控制为0-270° 对应占空比为500-2500
//	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, 100);
//	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_2, 100);
//	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, 500);
//	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_4, 500);	

	//第一个舵机数值越小越低 第二个舵机数值越大越低 
	//duoji_smooth(1450,2250);
  /* Infinite loop */
  for(;;)
  {
		if(dianji_flag==1)
		{
				if (g_usart_rx_sta & 0x8000)        /*串口6接收到了数据? */
				{
						len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
						printf("您发送的消息为:  ");
						HAL_UART_Transmit(&huart6, (uint8_t*)g_usart_rx_buf, len, 1000);     /* 发送接收到的数据 */
						if(g_usart_rx_buf[0]=='0')
						{				
							char data[] = "0\r\n";  
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data), 1000);  // 发送数据告诉MP157
							trash_type=0;
							duoji_smooth(1500,2300);
						}
						else if(g_usart_rx_buf[0]=='1'&&trash_type==0)
						{
							char data[] = "1\r\n";  
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data), 1000);  // 发送数据告诉MP157
							trash_type=1;	
						}
						else if(g_usart_rx_buf[0]=='2')
						{
							char data[] = "2\r\n";  
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data), 1000);  // 发送数据告诉MP157
							trash_type=2;	
						}
						else if(g_usart_rx_buf[0]=='3')
						{
							char data[] = "3\r\n";  
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data), 1000);  // 发送数据告诉MP157
							trash_type=3;	
						}
						else if(g_usart_rx_buf[0]=='4')
						{
							char data[] = "4\r\n";  
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data), 1000);  // 发送数据告诉MP157
							trash_type=4;	
						}
						else if(g_usart_rx_buf[0]=='5')
						{
							char data[] = "5\r\n";  
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data), 1000);  // 发送数据告诉MP157
							trash_type=5;	
						}
						else if(g_usart_rx_buf[0]=='6')
						{
							char data[] = "6\r\n";  
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data), 1000);  // 发送数据告诉MP157
							trash_type=6;	
						}
						key_cnt = atoi((const char*)g_usart_rx_buf)/500;
						printf("\r\n");
						while(__HAL_UART_GET_FLAG(&huart6, UART_FLAG_TC) != SET);           /* 等待发送结束 */
						g_usart_rx_sta = 0;
				}
//				if (g_usart_rx_sta2 & 0x8000)        /*串口7接收到了数据? */
//				{
//						len2 = g_usart_rx_sta2 & 0x3fff;  /* 得到此次接收到的数据长度 */
//						HAL_UART_Transmit(&huart7, (uint8_t*)g_usart_rx_buf2, len2, 1000);     /* 发送接收到的数据 */
//						while(__HAL_UART_GET_FLAG(&huart7, UART_FLAG_TC) != SET);           /* 等待发送结束 */
//						g_usart_rx_sta2 = 0;
//				}
				
				if(trash_type==1)
				{
						//开盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOF, GPIO_PIN_0, 	GPIO_PIN_RESET);//电机正转
								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);  //关闭输入捕获
						
						//开盖完成 舵机倾斜
						duoji_smooth(1700,2450);
						
						//关盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOF, GPIO_PIN_0, 	GPIO_PIN_SET);//电机反
								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);  //关闭输入捕获
						trash_type=0;
				}
				else if(trash_type==2)
				{
						//开盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOF, GPIO_PIN_1, 	GPIO_PIN_RESET);//电机正转
								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);  //关闭输入捕获
						
						//开盖完成 舵机倾斜
						duoji_smooth(1300,2450);
						
						//关盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOF, GPIO_PIN_1, 	GPIO_PIN_SET);//电机反
								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);  //关闭输入捕获
						trash_type=0;
				}
				else if(trash_type==3)
				{
						//开盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim8, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_1);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, 	GPIO_PIN_RESET);//电机正转
								__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim8, TIM_CHANNEL_1);  //关闭输入捕获
						
						//开盖完成 舵机倾斜
						duoji_smooth(1000,2300);
						
						//关盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim8, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_1);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4 ,GPIO_PIN_SET);//电机反
								__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim8, TIM_CHANNEL_1);  //关闭输入捕获
						trash_type=0;
				}
				else if(trash_type==4)
				{
						//开盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim8, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 	GPIO_PIN_RESET);//电机正转
								__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim8, TIM_CHANNEL_2);  //关闭输入捕获
						
						//开盖完成 舵机倾斜
						duoji_smooth(1200,1800);
						
						//关盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim8, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 	GPIO_PIN_SET);//电机反
								__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim8, TIM_CHANNEL_2);  //关闭输入捕获
						trash_type=0;
				}
				else if(trash_type==5)
				{
						//开盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim5, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, 	GPIO_PIN_RESET);//电机正转
								__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim5, TIM_CHANNEL_1);  //关闭输入捕获
						
						//开盖完成 舵机倾斜
						duoji_smooth(2000,2300);
						
						//关盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim5, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12,GPIO_PIN_SET);//电机反
								__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim5, TIM_CHANNEL_1);  //关闭输入捕获
						trash_type=0;
				}
				else if(trash_type==6)
				{
						//开盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim5, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_2);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, 	GPIO_PIN_RESET);//电机正转
								__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_4, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_4, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim5, TIM_CHANNEL_2);  //关闭输入捕获
						
						//开盖完成 舵机倾斜
						duoji_smooth(2000,2000);
						
						//关盖
						start_time=0;
						__HAL_TIM_SET_CAPTUREPOLARITY(&htim5, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING); //开启输入捕获中断，设置下降沿触发中断
						HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_2);    //启动输入捕获
						while(start_time<=stop_time)
						{
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, 	GPIO_PIN_SET);//电机反
								__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_4, 25);//电机转动
						}
						__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_4, 57);//电机停
						HAL_TIM_IC_Stop_IT(&htim5, TIM_CHANNEL_2);  //关闭输入捕获
						trash_type=0;
				}
		}
		else if(dianji_flag==0)//无电机版本
		{
				if (g_usart_rx_sta & 0x8000)        /*串口6接收到了数据? */
				{
						len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
						printf("您发送的消息为:  ");
						HAL_UART_Transmit(&huart6, (uint8_t*)g_usart_rx_buf, len, 1000);     /* 发送接收到的数据 */
						if(g_usart_rx_buf[0]=='0')
						{				
							HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 	GPIO_PIN_RESET);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 	GPIO_PIN_SET);
							HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 	GPIO_PIN_SET);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 	GPIO_PIN_SET);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 	GPIO_PIN_SET);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, 	GPIO_PIN_SET);
//							char data[] = "1\r\n"; 
//							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data)-1, 1000);  // 发送数据告诉MP157
							duoji_smooth(1450,2350);

						}
						else if(g_usart_rx_buf[0]=='1')//ok 纸巾
						{
							HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 	GPIO_PIN_RESET);
							duoji_smooth(1600,2490);
							char data[] = "1\r\n";  
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data)-1, 1000);  // 发送数据告诉MP157
						}
						else if(g_usart_rx_buf[0]=='2')//ok 易拉罐
						{
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 	GPIO_PIN_RESET);
							char data[] = "2\r\n"; 
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data)-1, 1000);  // 发送数据告诉MP157
							duoji_smooth(1350,2480);
						}
						else if(g_usart_rx_buf[0]=='3')//ok 有害
						{
							HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 	GPIO_PIN_RESET);
							char data[] = "3\r\n"; 
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data)-1, 1000);  // 发送数据告诉MP157
							duoji_smooth(1000,2300);
						}
						else if(g_usart_rx_buf[0]=='4')//ok 厨余
						{
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, 	GPIO_PIN_RESET);
							char data[] = "4\r\n"; 
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data)-1, 1000);  // 发送数据告诉MP157
							duoji_smooth(1200,1800);
						}
						else if(g_usart_rx_buf[0]=='5')//ok 纸箱
						{
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 	GPIO_PIN_RESET);
							char data[] = "5\r\n"; 
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data)-1, 1000);  // 发送数据告诉MP157
							duoji_smooth(2000,2300);
						}
						else if(g_usart_rx_buf[0]=='6')//ok 其他
						{
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 	GPIO_PIN_RESET);
							char data[] = "6\r\n"; 
							HAL_UART_Transmit(&huart7, (uint8_t*)data, sizeof(data)-1, 1000);  // 发送数据告诉MP157
							duoji_smooth(1800,2000);
						}
						key_cnt = atoi((const char*)g_usart_rx_buf)/500;
						printf("\r\n");
						while(__HAL_UART_GET_FLAG(&huart6, UART_FLAG_TC) != SET);           /* 等待发送结束 */
						g_usart_rx_sta = 0;
				}
		}

	}
}
/* USER CODE BEGIN Application */
//舵机平滑运动
int currentPosition_D=1450,currentPosition_L=2350,increment=1,delayTime=3;
void duoji_smooth(int targetPosition_D,int targetPosition_L)
{
	while(currentPosition_L!=targetPosition_L||currentPosition_D!=targetPosition_D)
	{
		if (currentPosition_L < targetPosition_L)
    {
      currentPosition_L += increment;
      if (currentPosition_L > targetPosition_L)
      {
        currentPosition_L = targetPosition_L;
      }
    }
    else if (currentPosition_L > targetPosition_L)
    {
      currentPosition_L -= increment;
      if (currentPosition_L < targetPosition_L)
      {
        currentPosition_L = targetPosition_L;
      }
    }
		
		if (currentPosition_D < targetPosition_D)
    {
      currentPosition_D += increment;
      if (currentPosition_D > targetPosition_D)
      {
        currentPosition_D = targetPosition_D;
      }
    }
    else if (currentPosition_D > targetPosition_D)
    {
      currentPosition_D -= increment;
      if (currentPosition_D < targetPosition_D)
      {
        currentPosition_D = targetPosition_D;
      }
    }
		
		__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, currentPosition_D);
		__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_2, currentPosition_L);
    HAL_Delay(delayTime);
	}
}

//输入捕获回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

	if(htim == &htim2||htim == &htim5||htim == &htim8)
	{
	   start_time++;
		HAL_GPIO_WritePin(GPIOE, LED2_Pin, GPIO_PIN_RESET);
	}
		
} 

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
