#include "stm32f10x.h" 
#include "stm32f10x_iwdg.h"  // 添加这一行
#include "delay.h"
#include "OLED.h"
#include "OELD_Data.h"
#include "FMQ.h"
#include "MQ2.h"
#include "dht11.h"
#include "key.h"
#include "ad.h"
#include "PWM.h"
#include "beep.h"
#include "MyUSART.H"
#include "esp.h"
#include "Servo.h"
#include "MQ7.h"
#include "stepmotor.h"
#include <stdio.h>
#include <string.h>
static uint8_t error_count = 0;
#define MAX_ERROR_COUNT 3  // 最大错误次数

// 变量定义
int MQ7_Value;//adc值
uint16_t fire,y,r,w,h,Y,W,R;
float temp_one,shi1;//电压值
int MQ2_Value;              
DHT11_Data_TypeDef  DHT11_Data;
// 控制标志位

//uint16_t key = 0, flag1 = 0, flag2 = 0, flag3 = 0,flag5 = 0,flag6 = 0,AD=0,wen1,A=0,tem=28,yan=70,ran=60,Judge,cnt;
// 各种标志位和控制变量
uint16_t key = 0;     // 按键值存储变量
uint16_t flag1 = 0;   // 模式切换标志位
uint16_t flag2 = 0;   // 通用标志位
uint16_t flag3 = 0;   // 设置模式标志位
uint16_t flag5 = 0;   // 燃气检测标志位
uint16_t flag6 = 0;   // 舵机控制标志位
uint16_t AD = 0;      // AD转换值存储
float wen1;        // 温度值存储
uint16_t A = 0;       // 通用计数器
uint16_t tem = 28;    // 温度阈值（28度）
uint16_t yan = 70;    // 烟雾阈值（70%）
uint16_t ran = 60;    // 燃气阈值（60%）
uint16_t Judge;       // WiFi连接判断标志
uint16_t cnt;         // 计数器
// DHT11温湿度传感器相关变量
uint8_t temp_int;     // 温度整数部分
uint8_t temp_deci;    // 温度小数部分
uint8_t humi_int;     // 湿度整数部分
uint8_t humi_deci;    // 湿度小数部分
// 远程控制标志位（'0'表示关闭，'1'表示开启）
uint8_t yan1=0;// 烟雾阈值增加控制位
uint8_t yan2=0;// 烟雾阈值减少控制位
uint8_t feng=0;// 风扇控制位
uint8_t wena=0;// 温度阈值增加控制位
uint8_t wenb=0;// 温度阈值减少控制位
uint8_t shui=0;// 水泵控制位
uint8_t bao=0; // 报警器控制位
uint8_t window=0;// 窗户（舵机）控制位
uint8_t rana=0;// 燃气co阈值增加控制位
uint8_t ranb=0;  // 燃气co阈值减少控制位
uint8_t display_page = 0;
static uint8_t auto_fan_state = 0;    // 风扇自动控制状态
static uint8_t auto_pump_state = 0;   // 水泵自动控制状态
#define WZ DHT11_Data.temp_int	
#define WX DHT11_Data.temp_deci
#define SZ DHT11_Data.humi_int	
#define SX DHT11_Data.humi_deci



void IWDG_Init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  // 使能写入功能
    IWDG_SetPrescaler(IWDG_Prescaler_256);        // 设置256分频
    IWDG_SetReload(1562);                         // 设置重装载值（约10秒）
    IWDG_ReloadCounter();                         // 喂狗
    IWDG_Enable();                                // 使能看门狗
}



 // 远程控制设备处理函数
static void Handle_Remote_Control(void)
{
    // 使用静态变量记录上一次的状态
    static uint8_t prev_feng = 0;
    static uint8_t prev_shui = 0;
    static uint8_t prev_fa = 0;
    static uint8_t prev_bao = 0;
    uint8_t state_changed = 0;  // 使用uint8_t替代bool

    // 风扇控制
    if(feng != prev_feng) {
        if(feng == 1) {
            fengkai();
            auto_fan_state = 0;  // 取消自动控制状态
        } else {
            fengguan();
        }
        prev_feng = feng;
        state_changed = 1;
    }

    // 水泵控制
    if(shui != prev_shui) {
        if(shui == 1) {
            shuikai();
            auto_pump_state = 0;  // 取消自动控制状态
        } else {
            shuiguan();
        }
        prev_shui = shui;
        state_changed = 1;
    }

    // 蜂鸣器控制
    if(bao != prev_bao) {
        BEEP = bao;
        prev_bao = bao;
        state_changed = 1;
    }

    // 窗户（舵机）控制
    if(window != prev_fa) {
        if(window == 1) {
            Servo_SetAngle(180);
            flag6 = 1;
        } else {
            Servo_SetAngle(0);
            flag6 = 0;
        }
        prev_fa = window;
        state_changed = 1;
    }

    // 阈值远程调节
    if(wena == 1) {
        if(tem < 40) {
            tem++;
            state_changed = 1;
        }
    }
    if(wenb == 1) {
        if(tem > 20) {
            tem--;
            state_changed = 1;
        }
    }
    
    if(yan1 == 1) {
        if(yan < 80) {
            yan++;
            state_changed = 1;
        }
    }
    if(yan2 == 1) {
        if(yan > 20) {
            yan--;
            state_changed = 1;
        }
    }
    
    if(rana == 1) {
        if(ran < 80) {
            ran++;
            state_changed = 1;
        }
    }
    if(ranb == 1) {
        if(ran > 20) {
            ran--;
            state_changed = 1;
        }
    }

    // 如果状态发生改变且WiFi连接正常，立即上报
    if(state_changed && !Judge) {
        Esp_PUB();
        cnt = 0;
    }
}





/***********************************************
 * 显示函数优化（保持原有功能）
 ***********************************************/
static void Update_Display_Content(void)
{
    switch(display_page)
    {
        case 0:
            OLED_Clear();
            OLED_ShowChinese(0, 0, "温度:");
            OLED_ShowNum(40, 0, WZ, 2, OLED_8X16);
            OLED_ShowString(56, 0, ".", OLED_8X16);
            OLED_ShowNum(64, 0, WX, 1, OLED_8X16);
            OLED_ShowString(72, 0, "C", OLED_8X16);
            
            OLED_ShowChinese(0, 16, "湿度:");
            OLED_ShowNum(40, 16, SZ, 2, OLED_8X16);
            OLED_ShowString(56, 16, ".", OLED_8X16);
            OLED_ShowNum(64, 16, SX, 1, OLED_8X16);
            OLED_ShowString(72, 16, "%", OLED_8X16);
            
            OLED_ShowString(95, 0, "P:1/3", OLED_8X16);
            break;

        case 1:
            OLED_Clear();
            OLED_ShowChinese(0, 0, "烟雾:");
            OLED_ShowNum(40, 0, MQ2_Value, 3, OLED_8X16);
            
            OLED_ShowString(0, 16, "CO:", OLED_8X16);
            OLED_ShowNum(32, 16, MQ7_Value, 3, OLED_8X16);
            
            OLED_ShowString(95, 0, "P:2/3", OLED_8X16);
            break;

        case 2:
            OLED_Clear();
            OLED_ShowChinese(0, 0, "火:");
            OLED_ShowString(40, 0, fire ? "WARNING" : "NORMAL", OLED_8X16);
            
            OLED_ShowString(0, 16, feng==1 ? "Fan:ON" : "Fan:OFF", OLED_8X16);
            OLED_ShowString(64, 16, shui==1 ? "Pump:ON" : "Pump:OFF", OLED_8X16);
            
            OLED_ShowString(95, 0, "P:3/3", OLED_8X16);
            break;
    }
}

static void Update_Sensor_Data(void)
{
			// 在循环开始处更新温湿度值
    if(Read_DHT11(&DHT11_Data) == SUCCESS)
    {
        float wendu = DHT11_Data.temp_int + DHT11_Data.temp_deci/10.0;
        float shidu = DHT11_Data.humi_int + DHT11_Data.humi_deci/10.0;
        wen1 = wendu;
        shi1 = shidu;
			// 获取传感器数据
        fire = IR_FireData();
        MQ2_Value=Get_Adc_Average1(ADC_Channel_3,10)*100/4095;//模拟烟雾浓度的值
        MQ7_Value=Get_Adc_Average1(ADC_Channel_1,10)*100/4095;//模拟co浓度的值
       
			
    }
	
}
/***********************************************
 * 按键处理优化
 ***********************************************/
static void Process_Key_Events(void)
{
    key = KEY_Scan(0);
    
    if(key)
    {
        switch(key)
        {
            case KEY0_PRES:  // 风扇+窗户控制
                auto_fan_state = 0;
                feng ^= 1;
                window = feng;
                feng ? (fengkai(), Servo_SetAngle(180)) : 
                      (fengguan(), Servo_SetAngle(0));
                // 状态改变后直接上报
                if(!Judge)
                {
                    Esp_PUB();
                    cnt = 0;
                }
                break;

            case KEY1_PRES:  // 水泵控制
                auto_pump_state = 0;
                shui ^= 1;
                shui ? shuikai() : shuiguan();
                // 状态改变后直接上报
                if(!Judge)
                {
                    Esp_PUB();
                    cnt = 0;
                }
                break;

            case KEY2_PRES:  // 模式切换
                flag3 = (flag3 == 4) ? 0 : 4;
                break;

            case KEY3_PRES:  // 页面切换
                if(flag3 != 4)
                    display_page = (display_page + 1) % 3;
                break;
        }
    }
}
/***********************************************
 * 自动控制系统优化
 ***********************************************/
static void Auto_Control_System(void)
{
    if(fire) {
        BEEP = 1;  // 火灾报警最高优先级
        if(!auto_pump_state) {
					   fire = 1;
            shuikai(); // 自动开启水泵
            auto_pump_state = 1;
            shui = 1;
            // 状态改变后直接上报
            if(!Judge)
            {
                Esp_PUB();
                cnt = 0;
            }
        }
    } 
    else if(MQ2_Value > yan || MQ7_Value > ran || DHT11_Data.temp_int > tem) {
        BEEP = 1;
        if(!auto_fan_state) {
            fengkai();
            Servo_SetAngle(180);
            auto_fan_state = 1;
            feng = 1;
            window = 1;
            // 状态改变后直接上报
            if(!Judge)
            {
                Esp_PUB();
                cnt = 0;
            }
        }
    } 
    else {
        BEEP = 0;
        if(auto_pump_state) {
            shuiguan(); 
            auto_pump_state = 0;
            shui = 0;
					  fire = 0;
            // 状态改变后直接上报
            if(!Judge)
            {
                Esp_PUB();
                cnt = 0;
            }
        }
        if(auto_fan_state) {
            fengguan();
            Servo_SetAngle(0); 
            auto_fan_state = 0;
            feng = 0;
            window = 0;
            // 状态改变后直接上报
            if(!Judge)
            {
                Esp_PUB();
                cnt = 0;
            }
        }
    }
}
	

int main(void)
{   
	    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    MyUSART_Init();
    AD_Init();
    Adc_Init();
    OLED_Init();
    mfq_Init();
    BEEP_Init();
    KEY_Init();
    DHT11_GPIO_Config();
    Adc_Init1();
    MOTOR_Init();
    Servo_Init();
	 IWDG_Init();  // 添加这一行，初始化看门狗
    // 尝试连接WiFi，但不阻塞程序
    Judge = esp_Init();
    while(1)
    {  

         IWDG_ReloadCounter();  // 添加这一行，定期喂狗
			
			 // 更新传感器数据
        Update_Sensor_Data();
        
        // 处理按键事件
        Process_Key_Events();
        
        // 自动控制系统
        Auto_Control_System();
       
       
		        // WiFi数据上报
       // 在main函数的while(1)循环中：
if(!Judge)  // WiFi连接正常
{
    cnt++;
    if(cnt >= 6)  // 缩短发送间隔
    {
        if(Esp_PUB() == 1)  // 发送失败
        {
            Judge = esp_Init();  // 重新初始化
        }
        cnt = 0;
    }
}

			 /* 显示系统 */
        if(flag3 == 4)
        {
            OLED_Clear();
            OLED_ShowChinese(0, 0, "远程");
            OLED_ShowChinese(0, 16, "温度:");
            OLED_ShowNum(40, 16, tem, 2, OLED_8X16);
            OLED_ShowString(64, 16, "C", OLED_8X16);
            OLED_ShowChinese(0, 32, "烟雾:");
            OLED_ShowNum(40, 32, yan, 2, OLED_8X16);
            OLED_ShowString(64, 32, "%", OLED_8X16);
            OLED_ShowString(0, 48, "CO:", OLED_8X16);
            OLED_ShowNum(32, 48, ran, 2, OLED_8X16);
            OLED_ShowString(56, 48, "%", OLED_8X16);
        }
        else
        {
            Update_Display_Content();
        }
        

				Handle_Remote_Control();

        
	
        

        



// 远程控制处理（放在主循环中）
   /*    if(flag3 == 4)
     {
    // 温度阈值远程调节
       if(wena == 1)
       {
        tem = tem + 1;
        if(tem > 40) tem = 40;
       }
        if(wenb == 1)
       {
           tem = tem - 1;
           if(tem < 20) tem = 20;
       }
    
       // 烟雾阈值远程调节
       if(yan1 == 1)
       {
           yan = yan + 1;
          if(yan > 80) yan = 80;
       }
       if(yan2 == 1)
       {
           yan = yan - 1;
            if(yan < 20) yan = 20;
       }
    
       // CO阈值远程调节
       if(rana == 1)
       {
           ran = ran + 1;
           if(ran > 80) ran = 80;
       }
       if(ranb == 1)
      {
           ran = ran - 1;
           if(ran < 20) ran = 20;
       }
    
       // 远程设备控制（保持原有功能）
       if(feng == 1)
           fengkai();
       else if(feng == 0)
           fengguan();
        
       if(shui == 1)
          shuikai();
       else if(shui == 0)
           shuiguan();
        
       if(bao == 1)
        BEEP = 1;
       else if(bao == 0)
           BEEP = 0;
        
        if(window == 1 && flag6 == 0)
       {
           Servo_SetAngle(180);
           flag6 = 1;
       }
       else if(window == 0 && flag6 == 1)
       {
          Servo_SetAngle(0);
           flag6 = 0;
       }
   }
       

        */

        
        OLED_Update();
        Delay_ms(100);
    }
}
