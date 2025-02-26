#include "FMQ.h"
#include "stm32f10x.h"   
void mfq_Init(void)	// 蜂鸣器初始化
{
	 // 配置PB0为推挽输出
	GPIO_InitTypeDef  GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}

void fmq(float MQ2_Value)  // 根据气体浓度控制报警
{
		if(MQ2_Value > 20)    // 当气体浓度超过20%时触发报警
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_0);//低电平触发
    }
		else
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_0);  // 高电平关闭蜂鸣器
		}
}
