
#ifndef __BSP_PS2_H__
#define __BSP_PS2_H__

#include <stdint.h>

#define PS2_MOSI_PIN	GPIO_PIN_7
#define PS2_MOSI_PORT	GPIOA


#define PS2_MISO_PIN	GPIO_PIN_6
#define PS2_MISO_PORT	GPIOA



#define PS2_CS_PIN		GPIO_PIN_9
#define PS2_CS_PORT		GPIOB


#define PS2_SCK_PIN		GPIO_PIN_3
#define PS2_SCK_PORT	GPIOB



#define DI   HAL_GPIO_ReadPin(PS2_MISO_PORT,PS2_MISO_PIN)					//输入

#define DO_H HAL_GPIO_WritePin(PS2_MOSI_PORT, PS2_MOSI_PIN, GPIO_PIN_SET)//命令位高
#define DO_L HAL_GPIO_WritePin(PS2_MOSI_PORT, PS2_MOSI_PIN, GPIO_PIN_RESET)//命令位低

#define CS_H HAL_GPIO_WritePin(PS2_CS_PORT, PS2_CS_PIN, GPIO_PIN_SET)//CS拉高
#define CS_L HAL_GPIO_WritePin(PS2_CS_PORT, PS2_CS_PIN, GPIO_PIN_RESET)//CS拉低

#define CLC_H HAL_GPIO_WritePin(PS2_SCK_PORT, PS2_SCK_PIN, GPIO_PIN_SET)//时钟拉高
#define CLC_L HAL_GPIO_WritePin(PS2_SCK_PORT, PS2_SCK_PIN, GPIO_PIN_RESET)//时钟拉低

//These are our button constants
#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2          9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_GREEN       13
#define PSB_RED         14
#define PSB_BLUE        15
#define PSB_PINK        16
#define PSB_TRIANGLE    13
#define PSB_CIRCLE      14
#define PSB_CROSS       15
#define PSB_SQUARE      16



//These are stick values
#define PSS_RX 5                //右摇杆X轴数据
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8


void PS2_Init(void);
void PS2_ClearData(void);
unsigned char PS2_DataKey(void);
void app_ps2_deal(void);
	
extern uint8_t Data[9];
extern int key_sta,key_cnt,key_cnt_L1,key_cnt_L2,key_cnt_R1,key_cnt_R2;

#endif
