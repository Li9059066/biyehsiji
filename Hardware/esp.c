#include "stm32f10x.h"                  // Device header
#include "MyUSART.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "OLED.H"


#define WZ DHT11_Data.temp_int	
#define WX DHT11_Data.temp_deci
#define SZ DHT11_Data.humi_int	
#define SX DHT11_Data.humi_deci
extern float wen1;
extern float shi1;

extern int MQ2_Value;
extern int MQ7_Value;
extern uint8_t yan1;
extern uint8_t yan2;
extern uint8_t feng;
extern uint8_t wena;
extern uint8_t wenb;
extern uint8_t shui;
extern uint8_t bao;
extern uint8_t fa;
extern uint8_t rana;
extern uint8_t ranb;
extern char RECS[250];
// WiFi和MQTT配置参数
const char* WIFI ="1234";               // WiFi名称
const char* WIFIASSWORD="123456789";    // WiFi密码
const char* ClintID="123";              // 设备ID
const char* username="7ingkW90fd";      // 产品id
const char* passwd="version=2018-10-31&res=products%2F7ingkW90fd&et=1803715797&method=sha1&sign=%2BdmILYSTZUAL2aGPlzmmXFj8Cuo%3D";//密钥
const char* Url="mqtts.heclouds.com";  // 云MQTT服务器
const char* pubtopic="$sys/7ingkW90fd/123/thing/property/post";    // 发布主题
const char* subtopic="$sys/7ingkW90fd/123/thing/property/post/reply";  // 订阅主题
const char* func1="temperature";  // 温度
const char* func2="humidity";  // 湿度
const char* func3="yan";  // 烟雾
const char* func4="feng"; // 风扇
const char* func5="yan1"; // 烟雾1
const char* func6="yan2";	 // 烟雾2
const char* func7="wen1";
const char* func8="wen2";
const char* func9="shui";
const char *func10="bao";
const char *func11="fa";
const char *func12="ran";
const char *func14="ran1";
const char *func15="ran2";
// 添加一个简单的错误计数器
static uint8_t error_count = 0;
#define MAX_ERROR_COUNT 3  // 最大错误次数

int fputc(int ch,FILE *f )   //printf重定向  
{
	USART_SendData(USART1,(uint8_t)ch);
	while(USART_GetFlagStatus (USART1,USART_FLAG_TC) == RESET);
	return ch;
}
char esp_Init(void)
{
	memset(RECS,0,sizeof(RECS));
	printf("AT+RST\r\n");  //重启
	Delay_ms(2000);
	
	memset(RECS,0,sizeof(RECS));
	printf("ATE0\r\n");    //关闭回显
	Delay_ms(10);
	if(strcmp(RECS,"OK"))
		return 1;
	
	printf("AT+CWMODE=1\r\n"); //Station模式
	Delay_ms(1000);
	if(strcmp(RECS,"OK"))
		return 2;
	
	memset(RECS,0,sizeof(RECS));
	printf("AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI,WIFIASSWORD); //连接热点
	Delay_ms(2000);
	if(strcmp(RECS,"OK"))
		return 3;
	
	memset(RECS,0,sizeof(RECS));
	printf("AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",ClintID,username,passwd);//用户信息配置
	Delay_ms(10);
	if(strcmp(RECS,"OK"))
		return 4;
	
	memset(RECS,0,sizeof(RECS));
	printf("AT+MQTTCONN=0,\"%s\",1883,1\r\n",Url); //连接服务器
	Delay_ms(1000);
	if(strcmp(RECS,"OK"))
		return 5;
	
	printf("AT+MQTTSUB=0,\"%s\",1\r\n",subtopic); //订阅消息
	Delay_ms(500);
	if(strcmp(RECS,"OK"))			
		return 5;
	memset(RECS,0,sizeof(RECS));
	return 0;
}



    char Esp_PUB(void)
{
	// 如果错误次数太多，重新初始化
    if(error_count >= MAX_ERROR_COUNT)
    {
        esp_Init();
        error_count = 0;
        return 1;
    }
	
	
    // 1. 上报温度数据
    memset(RECS, 0, sizeof(RECS));
    printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%.1f\\}}}\",0,0\r\n",
           func1,wen1);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
        return 1;
    
    // 2. 上报湿度数据
    memset(RECS, 0, sizeof(RECS));
    printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%.1f\\}}}\",0,0\r\n",
           func2,shi1);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
        return 1;
    
    // 3. 上报烟雾数据
    memset(RECS, 0, sizeof(RECS));
    printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",
           func3,MQ2_Value);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
        return 1;
    
    // 4. 上报CO数据
    memset(RECS, 0, sizeof(RECS));
    printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",
           func12,MQ7_Value);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
        return 1;
    
    // 5. 上报风扇状态
    memset(RECS, 0, sizeof(RECS));
    printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",
           func4,feng);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
        return 1;
    
    // 6. 上报水泵状态
    memset(RECS, 0, sizeof(RECS));
    printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",
           func9,shui);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
        return 1;

    return 0;
}
    


//数据上报实现功能：esp发送消息
//参数：无
//返回值：0：发送成功；1：发送失败
/*char Esp_PUB(void)
{
// 温度数据上报
    memset(RECS, 0, sizeof(RECS));
	
printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%.1f\\}}}\",0,0\r\n",func1,wen1);
	Delay_ms(200);//延时等待数据接收完成
	if(strcmp(RECS,"ERROR")==0)
		return 1;
	*/

 // 湿度数据上报
	/*memset(RECS, 0, sizeof(RECS));
printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%.1f\\}}}\",0,0\r\n",func2,shi1);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
    {
        return 1;
    }
    
    memset(RECS, 0, sizeof(RECS));
    
   // 烟雾数据上报
     printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",func3,MQ2_Value);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
    {
        return 1;
    }
    
    memset(RECS, 0, sizeof(RECS));
    
     // CO数据上报
    printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",func12,MQ7_Value);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
    {
        return 1;
    }
		 memset(RECS, 0, sizeof(RECS));
    
     // CO数据上报
    printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",func12,MQ7_Value);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
    {
        return 1;
    }
		 memset(RECS, 0, sizeof(RECS));
    
     // CO数据上报
    printf("AT+MQTTPUB=0,\"$sys/7ingkW90fd/123/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",func12,MQ7_Value);
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
    {
        return 1;
    }
//	printf("AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.event.property.post\\\"\\,\\\"params\\\":{\\\"%s\\\":%d\\,\\\"%s\\\":%f\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\}}\",0,0\r\n",
//	pubtopic,func1,wen1,func2,shi1,func3,MQ2_Value,func4,deng1,func5,guang1,func6,guang2,func7,shi11,func8,shi22,func9,duoji);
	 // 第一组数据上报：环境数据
	printf("AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.event.property.post\\\"\\,\\\"params\\\":{\\\"%s\\\":%d\\,\\\"%s\\\":%f\\,\\\"%s\\\":%f\\,\\\"%s\\\":%d\\}}\",0,0\r\n",
	pubtopic,// 发布主题
	func1,wen1,// 温度
	func2,shi1,// 湿度
	func3,MQ2_Value,// MQ2烟雾值
	func12,MQ7_Value// MQ7一氧化碳值\\\"%s\\\":{\\\"value\\\":%d\\}\\\"%s\\\":{\\\"value\\\":%d\\}}}
	);
	// 第二组数据上报：设备状态
     printf("AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.event.property.post\\\"\\,\\\"params\\\":{"
	      "\\\"%s\\\":%d\\,"     // 风扇状态
        "\\\"%s\\\":%d\\,"     // 烟雾传感器1
        "\\\"%s\\\":%d\\,"     // 烟雾传感器2
        "\\\"%s\\\":%d\\,"     // 温度传感器1
        "\\\"%s\\\":%d\\,"     // 温度传感器2
        "\\\"%s\\\":%d\\,"     // 水浸传感器
        "\\\"%s\\\":%d\\,"     // 报警器
        "\\\"%s\\\":%d\\,"     // 阀门
        "\\\"%s\\\":%d\\,"     // 燃气探测器1
        "\\\"%s\\\":%d\\}}"    // 燃气探测器2
        "\",0,0\r\n",
	 pubtopic, 
        func4,feng, func5,yan1, func6,yan2, func7,wena, func8,wenb,
        func9,shui, func10,bao, func11,fa, func14,rana, func15,ranb);
	//while(RECS[0]);//等待ESP返回数据
////	printf("AT+MQTTPUB=0,\"%s\",/thing/event/property/post","{\"params\":{\"Temperature\":27}}",0,0);
	//Delay_ms(200);//延时等待数据接收完成
	//if(strcmp(RECS,"ERROR")==0)
	//return 1;
	return 0;
}*/
void CommandAnalyse(void)
{
	// 检查是否收到MQTT消息
	if(strncmp(RECS,"+MQTTSUBRECV:",13)==0)
	{
		uint8_t i=0;
		while(RECS[i++] != '\0')             
		{
			 // 处理风扇控制命令
            if(strncmp((RECS+i),func4,4)==0)
			{ 
				while(RECS[i++] != ':');
				feng=RECS[i];
			}
			 // 处理烟雾传感器1控制
			if(strncmp((RECS+i),func5,4)==0)
			{
				while(RECS[i++] != ':');       
				yan1=RECS[i];
			}
			 // 传感器控制
			if(strncmp((RECS+i),func6,4)==0)
			{
				while(RECS[i++] != ':');
				yan2=RECS[i];
			}
			 // 处理报警器控制
			if(strncmp((RECS+i),func7,4)==0)
			{
				while(RECS[i++] != ':');
				wena=RECS[i];
			}
			// 处理窗户控制
			if(strncmp((RECS+i),func8,4)==0)
			{
				while(RECS[i++] != ':');
				wenb=RECS[i];
			}
			
			if(strncmp((RECS+i),func9,4)==0)
			{
				while(RECS[i++] != ':');
				shui=RECS[i];
			}
			if(strncmp((RECS+i),func10,3)==0)
			{
				while(RECS[i++] != ':');
				bao=RECS[i];
			}
				if(strncmp((RECS+i),func11,2)==0)
			{
				while(RECS[i++] != ':');
				fa=RECS[i];
			}
			if(strncmp((RECS+i),func14,4)==0)
			{
				while(RECS[i++] != ':');
				rana=RECS[i];
			}
			if(strncmp((RECS+i),func15,4)==0)
			{
				while(RECS[i++] != ':');
				ranb=RECS[i];
			}
		}
	}
}









