#ifndef __DEFINE_H
#define __DEFINE_H

#include "stdint.h"

#define SEND_REPORT_HTTP 0
#define RST_AF_HTTP_ERR  5

#define ESP32 1
#define AT_DEBUG 1
#define Reset_Pin 22
#define Reset_Pin_1 15
#define TYPE_NULL       0
#define TYPE_SIM800A    1
#define TYPE_SIM5300E   2
#define TYPE_UC15       3
#define TYPE_SIM7600CE   4
#define TYPE_A7600C   5
#define TYPE_SIM5320E   6

#define LED 13
#define LED_HIGH digitalWrite(LED,1)
#define LED_LOW digitalWrite(LED,0)

#define INIT_VALUE "4355"
#define IS_INIT_SIZE strlen(INIT_VALUE)
#define ID_DEFAULT 1
#define STATUS_WIFI_DEFALUT 1

#define SSID_DEFAULT "AhtLab"
#define PASS_DEFAULT "0941732379"
#define URL_DEFAULT "ibsmanage.com"
#define PHONE_DEFAULT "0933511989"
#define CALL_DEFAULT "sc:0933226630;st:0;se:0;mode:0;"
#define VERSION "ESP32SIM5300E103"

#define TIME_CONNECT 30000

#define Debug Serial

#define CALLING             1
#define LISTENING           2
#define IDLE                3

extern uint8_t call_incoming;
extern int Module_type;
extern char AT_Buff[1024];
extern int Sms_Process(char *info,char *content);

extern int answer;
extern char Http_res[256];
extern char URL_REQUEST[256];
extern char SimImei[20] ;// {0};
extern char ModuleImei[20] ;// {0};

extern char longitude[20];// {0};
extern char latitude[20];// {0};
extern char Lac[10];//{0};
extern char Ci[10];//{0};
extern char PhoneNum_Report[15];


extern int IsSendreport ;// 0;
extern int IsSendrequest ;//0;

extern uint8_t Http_err_cnt ;//0;
extern uint8_t wifi_err_cnt ;//0;
extern int Module_type ;// TYPE_NULL;

extern int statuscode;
extern int contentlength;

extern int ID_partner;
extern int Wifi_status;
extern char Wifi_Ssid[33];
extern char Wifi_Pass[65];
extern char Url[33];

extern int inPin;

extern int   Callrequest;
extern char  CallPhone[20];
extern int   Calltime;
extern int   Delaytime ;

extern char Moduletype[20];
extern char ModuleInfo[254];
extern char Gen_th[5];

extern uint32_t tot_listen_time;
extern uint32_t tot_call_time;
extern uint32_t tot_call_cnt;
extern uint32_t tot_listen_cnt;

extern char sms_phone_call[15];
extern uint8_t st;
extern uint8_t se;
extern uint32_t System_Mode;
extern uint32_t Http_Delay_time;
extern uint32_t Sms_delay_time;
extern uint32_t system_delay_time;
extern uint32_t Start_time_t;
extern uint8_t Cpinready;

extern hw_timer_t *timer;

extern uint8_t Status_Reg ;
extern uint8_t Status_Next;
extern uint8_t IsLedon ;
extern uint8_t sLed_cnt;
extern uint32_t Led_Time;

extern uint32_t tot_call_default_time;
extern uint32_t tot_call_default_cnt ;

extern char call_str[50];
extern int Http_err_code[256];
extern uint8_t Http_err_code_cnt;
/*-------------------user function-------------------------*/
void User_wifi_start(void);
void Module_detect(void);
int Modem_Lock_Band_3G();
void Get_info_module(void);
int AT_Get_Phone_Activity_Status(void);
int Wifi_is_connected(void);
int Wifi_Http_ota(char *url);
int Sms_Process(char *info,char *content);
int Get_value(char *des,char *scr,char *key);
int Wifi_Http_request(char *Url,char *rsp);
int Process_content_http(char *content);
void EEPROM_Process(void);
void Http_request(void);
void Process_result_from_http(void);
void Check_sim_ready(void);
void Watchdog_start(void);
void Send_report(char *sdt);
void Process_call(void);
int Sms_Send_result_call_default(char *sdt);
#endif
