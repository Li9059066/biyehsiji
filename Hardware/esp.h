#ifndef __esp_H
#define __esp_H
char esp_Init(void);
char Esp_PUB(void);
void CommandAnalyse(void);
// 全局变量声明
extern const char* WIFI;
extern const char* WIFIASSWORD;
extern const char* ClintID;
extern const char* username;
extern const char* passwd;
extern const char* Url;
extern const char* pubtopic;
extern const char* subtopic;
#endif
