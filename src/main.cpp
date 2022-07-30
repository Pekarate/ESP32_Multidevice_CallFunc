
#include <Arduino.h>
#include "Esp32_define.h"
#include "Esp32_ftcEeprom.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include "lwip/dns.h"
#include "lwip/netdb.h"


#include "esp_http_client.h" //idf

#include <Update.h>

#include "Esp32_AT_Hal.h"
#include "Esp32_AT_Common.h"
#include "Esp32_AT_http.h"
#include "Esp32_AT_sms_call.h"


char Version[]= "1000.0";
int answer;
uint8_t call_incoming =0;
char Http_res[256];
char URL_REQUEST[512];
char SimImei[20] = {0};
char ModuleImei[20] = {0};
char Lac[10]={0};
char Ci[10]={0};
char PhoneNum_Report[15];
int IsSendreport = 0;
int IsSendrequest =0;
uint8_t Http_err_cnt =0;
uint8_t wifi_err_cnt =0;
int Module_type = TYPE_NULL;
int statuscode;
int contentlength;
int ID_partner =0;
int Wifi_status =0;
char Wifi_Ssid[33];
char Wifi_Pass[65];

char sms_phone_call[15];
uint8_t st=0;
uint8_t se=0;

char Url[33];
int inPin = 14;

int   Callrequest =0;
char  CallPhone[20] = {0};
int   Calltime = 0;
int   Delaytime = 10000;

uint8_t Status_Reg  = IDLE;
uint8_t Status_Next = IDLE;
uint8_t IsLedon =0;
uint8_t sLed_cnt =0;
uint32_t Led_Time=0;

uint32_t Http_Delay_time = 0;
uint32_t System_Mode =0;
uint32_t Sms_delay_time =0;
uint32_t system_delay_time=0;
uint32_t Start_time_t =0;
uint32_t Send_report_time =0;


uint32_t tot_listen_time=0;
uint32_t tot_call_time=0;
uint32_t tot_call_cnt =0;
uint32_t tot_listen_cnt=0;

char Moduletype[20];
char ModuleInfo[254];
char Gen_th[5];
char PhoneNum_Report_t[20];

char call_str[50];
#define LED_NET 33
#define LED_NET_HIGH digitalWrite(LED_NET,1)
#define LED_NET_LOW digitalWrite(LED_NET,0)

int Http_err_code[256] = {0};
uint8_t Http_err_code_cnt = 0;

char OTP_Number[MAX_OTP_BUFFER_LEN+1];
char New_Otp = 0;
int Http_Try =3;

char SmsNumber[15]={0};
char SmsContent[100]={0};
uint8_t Request_sendsms = 0;
uint8_t smsOTP_Try = 3;
void setup() {
  // put your setup code here, to run once:
  setCpuFrequencyMhz(80);
  
  Watchdog_start();
  
  Debug.begin(115200);
  pinMode(LED_NET,OUTPUT);
  LED_NET_LOW;
  delay(100);
  LED_NET_HIGH;
  Debug.printf("------vesion: %s---clock: %d ,APB: %d --times compile: '%s'------\n",VERSION,getCpuFrequencyMhz(),getApbFrequency(), __TIME__);
  delay(1000);
  EEPROM_Process();
  LED_NET_LOW;
  Debug.printf("ID_partner: %d; Url: %s\n",ID_partner,Url);
  LED_NET_HIGH;
  #ifdef USE_WIFI
  User_wifi_start();
  #endif
  AT_Device_Init(115200);
  LED_NET_LOW;
  delay(1000);
  AT_Call_SetAutoAnswer();

  LED_NET_HIGH;
  delay(1000);
  Module_detect();
  LED_NET_LOW;
  delay(10000);
  Check_sim_ready();
  LED_NET_HIGH;
  delay(1000);
  Modem_Lock_Band_3G();
  LED_NET_LOW;
  delay(1000);
  Get_info_module();
  LED_NET_HIGH;
  delay(100);
  LED_NET_LOW;
  delay(1000);
  ID_partner =34;
  AT_Call_SetAutoCLCC();
}
void loop() {
  LED_NET_HIGH;
  delay(100);
  Status_Next = IDLE;
  String gen=String(Gen_th);
  if (gen!="3G")
  {
    Modem_Lock_Band_3G();
    delay(1000);
  }
  if(!New_Otp)
  {
    AT_Get_Phone_Activity_Status();
    Process_call();
  }
  if(New_Otp)
  {
        
        sprintf(URL_REQUEST,"http://%s/Active?IDS=%s&IDM=%s&sms=%s",Url,SimImei,ModuleImei,OTP_Number);
        Http_request(3);
        if(answer != 200)
        {
          printf("fail send OTP: %s\r\n",OTP_Number);
          if(!smsOTP_Try)
          {
            New_Otp=1;
          }
          else
          {
            smsOTP_Try--;
          }
        }
        else
        {
          New_Otp=0;
          smsOTP_Try = 3;
          printf("Send OTP: %s Done\r\n",OTP_Number);
        }
  }
  if(((millis() > Http_Delay_time) && (System_Mode ==0)))
  {
      //Debug.println("---------start to send request to server ---------");
      delay(100);
      Start_time_t = millis();
        sprintf(URL_REQUEST,"http://%s/Active?IDS=%s&IDM=%s&G=%s&D=%s&P=%d&t=%d&c=200789&ver=%s",Url,SimImei,ModuleImei,Gen_th,Moduletype,ID_partner,Calltime,Version);
      Debug.printf("Get: %s\n",URL_REQUEST);
      Http_request(3);
      Calltime = 0;
      Process_result_from_http();
      //Debug.printf("-------------end request to server : %lu---------\n",millis() -Start_time_t);
      Http_Delay_time = millis() +30000;  //delay 15s
      delay(100);
  }
  if(millis() > Sms_delay_time)
  {
    Start_time_t = millis();
    //Debug.println("---------start process sms  ---------");
    //int a = AT_Sms_Getlist();     //doc toan bo tin nhan va xu ly
    AT_Sms_Getlist();
    //Debug.printf("has processed %d messages\n",a);
    Sms_delay_time = millis() + 5000; //delay 5s
    //Debug.printf("-----------end process sms :%lu ----------\n",millis() -Start_time_t);
  }
  if(Request_sendsms)
  {
      AT_Sms_Send(SmsNumber, SmsContent);
  }
  // if(millis()>3600000)
  // {
  //   ESP.restart();
  // }
}
