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
TaskHandle_t Task1;
const int wdtTimeout = 90000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;
void IRAM_ATTR resetModule() {
  ets_printf("-----reboot------\n");
  esp_restart();
}

void Watchdog_start(void)
{
  
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
  timerAlarmEnable(timer);   
  timerWrite(timer, 0);  
}

void Module_detect(void){
  Module_type = TYPE_NULL;
  int cnt =3;
  while(cnt --)
  {
    if(AT_GetInfo(ModuleInfo))
    {
      if(strstr(ModuleInfo,"SIM800"))
      {
        Module_type = TYPE_SIM800A;
        sprintf(Moduletype,"1"); 
        Debug.println("Detechted 800");
        break;
      }
      else if(strstr(ModuleInfo,"UC15"))
      {
        Module_type = TYPE_UC15;
        sprintf(Moduletype,"6");
        Debug.println("Detechted UC15");
        break;
      }
      else if(strstr(ModuleInfo,"SIM5300"))
      {
        Module_type = TYPE_SIM5300E;
        sprintf(Moduletype,"4");
        Debug.println("Detechted 5300");
        break;
      }
      else if(strstr(ModuleInfo,"SIM5320"))
      {
        Module_type = TYPE_SIM5300E;
        sprintf(Moduletype,"5");
        Debug.println("Detechted 5320");
        break;
      }
      else if(strstr(ModuleInfo,"SIM7600"))
      {
        Module_type = TYPE_SIM7600CE;
        sprintf(Moduletype,"7");
        Debug.println("Detechted 7600");
        At_Command((char *)"AT+CGSOCKCONT=1,\"IP\",\"myapn\"",(char *)"OK\r\n",5000);
        break;
      }
      else if(strstr(ModuleInfo,"A7600"))
      {
        Module_type = TYPE_A7600C;
        sprintf(Moduletype,"7");
        Debug.println("Detechted A7600");
        At_Command((char *)"AT+CGSOCKCONT=1,\"IP\",\"myapn\"",(char *)"OK\r\n",5000);
        break;
      }
      else
      {
        Debug.printf("Cant't not detect : %s\n",ModuleInfo);
      }
    }
    delay(3000);
  }
  if(Module_type == TYPE_NULL)
    ESP.restart();
}


#define LED_ON digitalWrite(LED,1)
#define LED_OFF digitalWrite(LED,0)
static void Btn_task( void * pvParameters ){
  pinMode(inPin, INPUT_PULLUP);
  pinMode(LED,OUTPUT);
  for(;;){
    
    if(!digitalRead(inPin))
    {
      delay(20);
      if(!digitalRead(inPin))
      {
        int start = millis();
        while (!digitalRead(inPin))
        {
          delay(1);
        }
        int t = millis() - start;
        for(uint8_t i=0; i<10;i++)
        {
          if(i%2)
            LED_ON;
          else
            LED_OFF;
          delay(50);
        }
        if(t < 3000)
        {
           IsSendreport =1;
        }
        else
        {
          for(int i=10;i>0;i--)
          {
            
            delay(1000);
          }
          esp_restart();
        }
        
      }
    }
    delay(1);
    if(millis() >Led_Time)
    {
      if(IsLedon)
      {  
        LED_OFF;
        IsLedon =0;
      }
      else
      {  
        LED_ON;
        IsLedon =1;
      }
      switch (Status_Next)
      {
      case IDLE:
        Led_Time = millis()+1000;  
        break;
      case CALLING:
        Led_Time = millis()+500;  
        break;
      case LISTENING:
        Led_Time = millis()+150;  
        break;
      
      }
    }
  } 
}
#ifdef USE_WIFI
void User_wifi_start(void)
{
    if(Wifi_status)
    {
        Debug.printf("Wifi_Ssid: %s;Wifi_Pass: %s\n",Wifi_Ssid,Wifi_Pass);
        WiFi.begin(Wifi_Ssid, Wifi_Pass);                         // Connect to WiFi network
        int numOfTry = 0;
        Debug.printf("connect to wifi: %s",Wifi_Ssid);
        while (WiFi.status() != WL_CONNECTED) {         // Wait for connection
            delay(500);
            Debug.print(".");
            numOfTry++;
            if(numOfTry >= 10) {
                break;
            }
        }
    }
    else
        Debug.printf("Turn off wifi\n");
}
#endif
int Modem_Lock_Band_3G()
{
  uint8_t cnt_t =5;
  if(Module_type == TYPE_SIM5300E)
  {
    if(At_Command((char *)"AT+CBAND?",(char *)"OK\r\n",5000)>0)
    {
      char band[20];
      if(AT_Getstring_index(band,AT_Buff,(char *)": ",0)>0)
      {
        Debug.printf(" modem runing : %s \n",band);
        if(strstr(band,"I_MODE"))
        {
            sprintf(Gen_th,"3G");
            return 1;
        }
      }
    }
    while (cnt_t --)
    {
      if(At_Command((char *)"AT+CBAND=\"UMTS_I_MODE\"",(char *)"OK\r\n",2000)>0)
      {
        Debug.println(" Lock Band done");
        sprintf(Gen_th,"3G");
        return 1;
      }
    }
  }
  if(Module_type == TYPE_SIM5320E)
  {
    if(At_Command((char *)"AT+CBAND?",(char *)"OK\r\n",5000)>0)
    {
      char band[20];
      if(AT_Getstring_index(band,AT_Buff,(char *)": ",0)>0)
      {
        Debug.printf(" modem runing : %s \n",band);
        if(strstr(band,"I_MODE"))
        {
            sprintf(Gen_th,"3G");
            return 1;
        }
      }
    }
    while (cnt_t --)
    {
      if(At_Command((char *)"AT+CBAND=\"UMTS_I_MODE\"",(char *)"OK\r\n",2000)>0)
      {
        Debug.println(" Lock Band done");
        sprintf(Gen_th,"3G");
        return 1;
      }
    }
  }
  else if(Module_type == TYPE_UC15)
  {
    At_Command((char *)"AT+QGBAND",(char *)"OK\r\n",2000);
    while (cnt_t --)
    {
      if(At_Command((char *)"AT+QCFG=\"band\",16",(char *)"OK\r\n",2000)>0)
      {
        sprintf(Gen_th,"3G");
        Debug.println(" Lock Band done");
        return 1;
      }
    }
  }
  else if(Module_type == TYPE_SIM7600CE)
  {
      At_Command((char *)"AT+CNMP?",(char *)"OK\r\n",2000);
    while (cnt_t --)
    {
      if(At_Command((char *)"AT+CNMP=14",(char *)"OK\r\n",2000)>0)
      {
        sprintf(Gen_th,"3G");
        Debug.println(" Lock Band done");
        return 1;
      }
    }

  }
  else if(Module_type == TYPE_A7600C)
  {
      At_Command((char *)"AT+CNMP?",(char *)"OK\r\n",2000);
    while (cnt_t --)
    {
      if(At_Command((char *)"AT+CNMP=14",(char *)"OK\r\n",2000)>0)
      {
        sprintf(Gen_th,"3G");
        Debug.println(" Lock Band done");
        return 1;
      }
    }
      
  }
  else if(Module_type == TYPE_SIM800A)
  {
      sprintf(Gen_th,"2G");
      return 1;
  }
  return 0;
}
void Get_info_module(void)
{
    int  cnt =3;
    while(1)
    {
        if(AT_Get_Module_IMEI(ModuleImei)>0)
        {
          Debug.println("ModuleImei: "+(String)ModuleImei);
          break;
        } 
        cnt--;
        if(cnt ==0)
          ESP.restart();
        delay(2000);
    }
    cnt =3;
    while(1)
    {
        if(AT_Get_Sim_IMEI(SimImei)>0)
        {
          Debug.println("SimImei: "+(String)SimImei);
          break;
        }
        cnt--;
        if(cnt ==0)
          ESP.restart();
        delay(2000);
    }
    if((strlen(ModuleImei)<9) || (strlen(SimImei)<9))
    {    
      ESP.restart();
    }
    At_Command1((char *)"AT+CSQ",(char *)"OK\r\n",2000);
}

int AT_Proces_Listen(uint32_t maxTime)
{
  uint8_t tmp[256] = {0};
  uint32_t Timetmp = millis() + maxTime;
  uint32_t StartTime = millis();
  uint32_t StartTime1 = millis()+1000;
  while(Timetmp > millis())
  {
    if(StartTime1 <millis() )
    {
      At_Command((char *)"AT+CLCC",(char *)"OK\r\n",2000);
      StartTime1 = millis()+1000;
    }
    // if(At_Command_nodebug((char *)"AT+CPAS",(char *)"OK\r\n",2000))
    // {
    //   if(strstr(AT_Buff,(char *)"CPAS: 0") || strstr(AT_Buff,(char *)"CPAS: 1")||strstr(AT_Buff,(char *)"CPAS: 2"))
    //   {
    //     printf("return with out endcall detected\r\n");
    //     return millis() - StartTime;
    //   }
    // }
    int s = AT_read_until(tmp,(char *)"CARRIER",256,5000);
    if(s > 0)
    {
      tmp[s] = 0;
      if(strstr((char *)tmp,"CARRIER")) //end call
      {
        At_Command((char *)"AT+CLCC",(char *)"OK\r\n",2000);
        return millis() - StartTime;
      }
    }
    if(CountUnexpected[NEWSMS])
    {
      AT_Sms_Getlist();
      if(New_Otp)
      {
        return millis() - StartTime;
      }
    }
    
  }
  return maxTime;
}

uint32_t StartTime1111 = 0;
int AT_Get_Phone_Activity_Status(void)
{
  if( Module_type == TYPE_A7600C)
  {

      if(At_Command_nodebug((char *)"AT+CLCC",(char *)"OK\r\n",2000))
      {
        if(strstr(AT_Buff,"+CLCC:"))
        {
          
        }
        return 1;
      }
      return 1;
  }  
  if(At_Command_nodebug((char *)"AT+CPAS",(char *)"OK\r\n",5000))
  {
    if(strstr(AT_Buff,(char *)"CPAS: 4")) //Call in progress
    { 
      
      Debug.printf("*");
      uint8_t cnt =5;
      if( Module_type == TYPE_A7600C)
      {
          return 4;
      }
      else
      {
              cnt =5;
              while(cnt --)
              {
                if(AT_Call_Hangup()>0)
                  break;
              }
              return 4;
      }
    }
    else if(strstr(AT_Buff,(char *)"CPAS: 0"))
      return 0;
    else if(strstr(AT_Buff,(char *)"CPAS: 1"))
      return 1;
    else if(strstr(AT_Buff,(char *)"CPAS: 2"))
      return 2;
    if(strstr(AT_Buff,(char *)"CPAS: 3"))   //Ringing
    {
      At_Command((char *)"AT+CLCC",(char *)"OK\r\n",2000);
      Debug.println("call detected");
      uint8_t tmp[256] = {0};
      uint8_t i =0;
      for( i=0;i<30;i++)
      {
        if(At_Command((char *)"ATA",(char *)"OK",1000) > 0)
        {
          break;
        }
        else
        {
          Debug.printf("ATA FAIL call: %s \n",AT_Buff);
        }
        delay(1000);
      }
      uint8_t incall =0;
      if(i>=10)
      {
        if(At_Command_nodebug((char *)"AT+CPAS",(char *)"OK\r\n",5000))
        {
          if(strstr(AT_Buff,(char *)"CPAS: 4"))
          {
            incall =1;
          }
        }
      }
      else
      {
        incall =1;
      }
      if(incall == 1)
      {
          Status_Next = LISTENING;
          Debug.println("call start");
          // Calltime = millis();

          // //AT_read_until(tmp,(char *)"CARRIER",256,20*60*1000);
          // Calltime = millis() - Calltime;
          Calltime = AT_Proces_Listen(20*60*1000);
          Calltime = Calltime/1000;
          tot_listen_time +=Calltime;
          tot_listen_cnt++;
          Debug.printf("time lissten: %d ms,%s\n",Calltime,tmp);
      }
      return 3;
    }
    else
      return 5;
  }
  return -1;
}
#ifdef USE_WIFI
int Wifi_is_connected(void)
{
  if(!Wifi_status)
  {
    Debug.printf(" Wifi  %s is turn off\n",Wifi_Ssid);
    return 0;
  }
  if(WiFi.status() != WL_CONNECTED) {
    Debug.printf("Lost connect try reconnect to  Wifi %s:\n",Wifi_Ssid);
    WiFi.begin(Wifi_Ssid, Wifi_Pass);             // Connect to WiFi network
    // isWifi = false;
    Wifi_status =0;
    return 0;
  } else {
    // isWifi = true;
    Debug.printf("Connected to  Wifi %s:\n",Wifi_Ssid);
    return 1;
  }
}
int Wifi_Http_ota(char *url)
{
  if(!Wifi_is_connected())
  {
    Debug.println("no wifi for update ota");
    return -1;
  }
  uint8_t Buff[1025];
  int recv=0;
  esp_http_client_config_t config = {
	        .url = url,
//	        .event_handler = _http_event_handler,
	  };
  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == NULL) {
	    Debug.printf("Failed to initialise HTTP connection");
      return -1;
	}
  esp_http_client_set_method(client,HTTP_METHOD_GET);
  if(esp_http_client_open(client,0) != ESP_OK)
  {
    Debug.printf("esp_http_client_open fail");
    return -1;
  }
  esp_http_client_fetch_headers(client);
  int content_length = esp_http_client_get_content_length(client);
  int code = esp_http_client_get_status_code(client);
  if(code == 200 && content_length >10000)
  {
      int tot=0;
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Debug);
      }
      else
      {
        Debug.printf("update.begin ok\n");
      }
      while (1)
      {
        recv = esp_http_client_read(client,(char *)Buff,1024);
        if(recv >0)
        {
          Buff[recv] =0;
          tot +=recv;
          if (Update.write(Buff, recv) != recv) {
            Update.printError(Debug);
      }
        }
        else
        {
          if(tot == content_length)
          {
            Debug.printf("client read data done: %d byte\n",tot);
            if (Update.end(true)) { //true to set the size to the current progress
              Debug.printf("Update Success: %u\nRebooting affer 10s\n", tot);
              for(uint8_t i=9;i>0;i--)
              {
                Debug.printf("Rebooting affer %d s\n",i);
                delay(1000);
              }
              ESP.restart();
            } else {
              Update.printError(Debug);
            }
          }
          else
          {
            Debug.printf("client read data error: %d byte != %d byte\n",tot,content_length);
          }
          break;
        }
      }
  }
  esp_http_client_close(client);
  esp_http_client_cleanup(client);
  return -1;
}
#endif
int Get_value(char *des,char *scr,char *key)
{
  char *start = strstr(scr,key);
  if(start == NULL)
    return -1;
  char *end  = strstr(start,";");
  if(end == NULL)
    return -1;
  int size = (int)end -(int)start-strlen(key); 
  if (size >0)
    memcpy(des,start+strlen(key),size);
  else
    return -2;
  des[size]=0;
  return size;
}
void Send_report(char *sdt)
{
  char tmp[256];
  // char call_default[50]={0};
  //AT_Get_Cellid(Lac,Ci);// long: %s;lat: %s;lac: %s;Ci: %s;-longitude,latitude,Lac,Ci,
  #ifdef USE_WIFI
  int s = sprintf(tmp,"[%lu]Id: %d;IDS=%s;n: IDM=%s;n: Url: %s;n: %s;Wifi : %d,%s,%s",millis(),ID_partner,SimImei,ModuleImei,Url,PhoneNum_Report,Wifi_status,Wifi_Ssid,Wifi_Pass);
  #else
  int s = sprintf(tmp,"[%lu]Id: %d;IDS=%s;n: IDM=%s;n: Url: %s;n: %s",millis(),ID_partner,SimImei,ModuleImei,Url,PhoneNum_Report);

  #endif
  Debug.printf("send %d byte: %s TO %s:",s,tmp,sdt);
  AT_Sms_Send(sdt,tmp);
}

int Get_OTP_Number(char *SMScontent,uint16_t len)
{
  int index= 0;
  for(int i=0;i< len;i++)
  {
      if((SMScontent[i]>('0'-1)) && (SMScontent[i]<('9'+1)))
      {
          OTP_Number[index] = SMScontent[i];
          index++;
      }
      else if((index == 6) && (SMScontent[i] =' '))
      {
         if(i == 6)
         {
             OTP_Number[6] =0;
             return 1;
         }
         else if(SMScontent[i-7] ==' ')
         {
             OTP_Number[6] =0;
             return 1;
         }
         else
         {
            index=0;
         }
      }
      else
      {
          index = 0;
      }
  }
  if(len>MAX_OTP_BUFFER_LEN)
  {
      len = MAX_OTP_BUFFER_LEN;
  }
  for(int i=0;i< len;i++)
  {
    if(!(((SMScontent[i]>='0') && (SMScontent[i]<='9'))||((SMScontent[i]>='A') && (SMScontent[i]<='Z'))||((SMScontent[i]>='a') && (SMScontent[i]<='z'))))
    {
      SMScontent[i]='_';  //remove 0x20
    }
  }
  memcpy(OTP_Number,SMScontent,len);
  OTP_Number[len]=0;
  return 0;
}

int Sms_Process(char *info,char *content)
{
  char valtmp[50];
  char number[30];
  int Num_ready =0;
  int send_report_sms =0;
  if(AT_Getstring_index(number,info,(char *)": ",2)>0)
  {
    Num_ready =1;
  }
  AT_Getstring_index(valtmp,info,(char *)": ",0); // sms index
  int index = atoi(valtmp);
  Debug.printf("AT_delete_Sms: %d\n",index);
  AT_Sms_Delele(index,0);
  if(strstr(content,"status?"))
  {
      //Send_report(number);
      send_report_sms =1;
  }
  else if(Get_value(valtmp,content,(char *)"server:")>0)
  {
    sprintf(Url,"%s",valtmp);
    Debug.printf("Url new: %s\n",Url);
    Eeprom_Set_Url(Url);
      send_report_sms =1;
  }
  else if(Get_value(valtmp,content,(char *)"reset")>0)
  {
      send_report_sms =1;
      ESP.restart();
  }
  else if(Get_value(valtmp,content,(char *)"p:")>0)
  {
    ID_partner = atoi(valtmp);
    Eeprom_Set_IDpartner(ID_partner);
    Debug.printf("ID_partner new: %d\n",Eeprom_Get_IDpartner());
      //sprintf(tmp,"Id: %d ;Url: %s;Wifi : %d,%s,%s",ID_partner,Url,Wifi_status,Wifi_Ssid,Wifi_Pass);
      send_report_sms =1;
    //return 1;
  }
  #ifdef USE_WIFI
  else if(Get_value(valtmp,content,(char *)"wifi:")>0)
  {
    char ssid_new[33];
    char pass_new[65];
    if(Get_value(ssid_new,content,(char *)"ssid:")>0)
      if(Get_value(pass_new,content,(char *)"pass:")>0)
      {
        sprintf(Wifi_Ssid,"%s",ssid_new);
        sprintf(Wifi_Pass,"%s",pass_new);
        Eeprom_Set_Ssid_Pass(Wifi_Ssid,Wifi_Pass);
        send_report_sms =1;
        if(strstr(valtmp,"on"))
        {
          Wifi_status =1;
          Eeprom_Set_Wifi_status(Wifi_status);
          Debug.printf("turn on Wifi: %s,%s\n",Wifi_Ssid,Wifi_Pass);
          WiFi.begin(Wifi_Ssid, Wifi_Pass);
        }
        else if(strstr(valtmp,"off"))
        {
          Wifi_status =0;
          Eeprom_Set_Wifi_status(0);
          Debug.printf("turn off Wifi new: %s,%s\n",Wifi_Ssid,Wifi_Pass);
          WiFi.disconnect();
        }
        else
          Debug.printf("sms set wifi error\n");
      }
  }
  #endif
  else if(strstr(content," OTP"))
  {

    Get_OTP_Number(content,strlen(content));
#if END_CALL_TO_SEND_OTP
    printf("send OTP: %s\r\n",OTP_Number);
    sprintf(URL_REQUEST,"http://%s/Active?IDS=%s&IDM=%s&sms=%s",Url,SimImei,ModuleImei,OTP_Number);
    Http_request(1);
    if(answer != 200)
    {
      printf("fail send OTP: %s\r\n",OTP_Number);
      New_Otp=1;
    }
    else
    {
      New_Otp = 0;
      printf("Send OTP: %s Done\r\n",OTP_Number);
    }
#else
    New_Otp = 1;
#endif
    
  }
  return -1;
}
int Wifi_Http_request(char *Url,char *rsp)
{
    HTTPClient http;    
    http.begin(Url);    
    int http_code = http.GET();             //Send the request
    String payload = http.getString();      //Get the response payload
    Debug.print("Http return code: ");
    Debug.println(http_code);  
    Debug.print("response: ");
    Debug.println(payload); 
    if(http_code == 200) {
      Debug.println(F("wifi request success"));
      sprintf(rsp,"%s",payload.c_str());
      http.end(); 
    }
    return http_code;
}
//-------

int Process_content_http(char *content)
{
  char tmp[50];
  int  tmp_t=0;
  if(Get_value(SmsNumber,content,(char *)"$sms:")>0)
  {
    if(Get_value(SmsContent,content,(char *)"$nd:")>0)
    {
      Request_sendsms = 1;
    }
  }
  else if(Get_value(UssdContent,content,(char *)"$ussd:")>0)
  {
    Request_sendussd = 1;
  }
  else if(Get_value(CallPhone,content,(char *)"$c:")>0)
  {
    if(Get_value(tmp,content,(char *)"$t:")>0)
    {
      tmp_t = atoi(tmp);
      if(tmp_t >0)
      {
        Calltime = tmp_t;
        return 1;
      }
    }
  }
  return -1;
}

void EEPROM_Process(void)
{
    Eeprom_Init();  
    if(!Eeprom_Is_Init_Value())
    {
      Debug.printf("set default all\n");
      #ifdef USE_WIFI
      Eeprom_Set_Ssid_Pass((char *)SSID_DEFAULT,(char *)PASS_DEFAULT);      //defaule wifi
      Eeprom_Set_Wifi_status(STATUS_WIFI_DEFALUT);
      Eeprom_Set_call_default((char *)CALL_DEFAULT);
      #endif
      Eeprom_Set_IDpartner(ID_DEFAULT);
      Eeprom_Set_Url((char *)URL_DEFAULT);    
      Eeprom_Set_phoneNum((char *)PHONE_DEFAULT); 

      Eeprom_Set_result_call(0,0);
      Set_Eeprom_Is_Init_Value();
    }
    ID_partner = Eeprom_Get_IDpartner();
    #ifdef USE_WIFI
    Wifi_status = Eeprom_Get_Wifi_status();
    Eeprom_Get_Ssid(Wifi_Ssid);
    Eeprom_Get_Password(Wifi_Pass);
    #endif
    Eeprom_Get_Url(Url);
    if(!strstr(Url,".com"))
    {
        sprintf(Url,URL_DEFAULT);
        Eeprom_Set_Url(Url); 
    }
    #ifdef USE_WIFI
    if((strlen(Wifi_Ssid) == 0) ||(strlen(Wifi_Pass) == 0))
    {
        sprintf(Wifi_Ssid,SSID_DEFAULT);
        sprintf(Wifi_Pass,PASS_DEFAULT);
        Eeprom_Set_Ssid_Pass(Wifi_Ssid,Wifi_Pass); 
    }
    #endif
}

void Process_call(void)
{
  if(System_Mode == 1 )
    Callrequest =2;
  if(Callrequest == 2)
  {
    if(se - st >1)
    {
      sprintf(CallPhone,sms_phone_call);
      Calltime = (rand() % (se - st + 1)) + st;
    }
    else
      Callrequest =0;
  }
  if(Callrequest > 0)
  {
      if(Callrequest == 1)
      {
        Debug.printf("Call from http request\n");
      }
      else if(Callrequest == 2)
      {
        Debug.printf("Call from eeprom request\n");
      }
      Debug.printf("start call: %s in %d s\n",CallPhone,Calltime);
      int ctry = 3;
      while(ctry --)
      {
        if(AT_Call_To(CallPhone) >0)   //thuc hien cuoc goi
            break;
        delay(1000);
      }
      if(!ctry) 
      {
        Debug.printf("call : %s fail \n",CallPhone);  // khong the thuc hien cuoc goi
        Check_sim_ready();
      }
      else  //call ok
      {
          Status_Next = CALLING;
          int res;
          if((Module_type == TYPE_SIM7600CE) || (Module_type == TYPE_A7600C) || (Module_type == TYPE_SIM5320E))
          {
            res = AT_SIM7600_call_Waitresult(Calltime);
          }
          else{
            delay(TIME_CONNECT);  // delay for connect
            res = AT_call_Waitresult(Calltime);
          }
          if(res >0)
          {
              Calltime = res/1000;
              Debug.printf("Call ok:%d s\n",Calltime);
              if(Callrequest >0)
              {
                tot_call_time +=Calltime;
                tot_call_cnt++;
              }
          }
          else
          {
            Debug.printf("Call fail: %d\n",res);
            Calltime= 0;
          }
      }
      Callrequest =0;
  }
}
void Http_request(int Try_times)
{
    answer = 0;
    Http_Try = Try_times;
    memset(Http_res,0,256);
    #ifdef USE_WIFI
    if(!Wifi_is_connected())
      {
    #endif
          Check_sim_ready();
          Debug.println("Requet over module sim");
          //
          if(Module_type == TYPE_UC15)
            answer= AT_UC15_HTTP_Get(URL_REQUEST,Http_res);
          else if((Module_type == TYPE_SIM7600CE)  || 
                  (Module_type == TYPE_SIM5320E))  
          {
            answer= AT_SIM7600_HTTP_Get(URL_REQUEST,Http_res);
          }  
          // else if(Module_type == TYPE_SIM5300E)
          // {
          //   answer= AT_SIM5300_HTTP_Get(URL_REQUEST,Http_res);
          // } 
          else //Module_type == TYPE_SIM5300E
          {  
            answer = AT_Http_Request(URL_REQUEST,Http_res);
          }
      #ifdef USE_WIFI
      }
      else
      {
          Debug.println("Requet over wifi");
          answer = Wifi_Http_request(URL_REQUEST,Http_res);
      }
      #endif
}



void Send_report_http_code(char *sdt)
{
  char tmp[256];
  sprintf(tmp,"%lu %d %d %d %d %d %d %d %d %d %d",millis(),Http_err_code[0],Http_err_code[1],Http_err_code[2],Http_err_code[3],Http_err_code[4],Http_err_code[5],Http_err_code[6],Http_err_code[7],Http_err_code[8],Http_err_code[9]);
  //Debug.printf("send %d byte: %s TO %s:",s,tmp,sdt);
  AT_Sms_Send(sdt,tmp);
}

void Process_result_from_http(void)
{
    if(answer ==200)
    {
        Calltime = 0; // reset time call
        Debug.printf("HTTP_request: %d-> %s",answer,Http_res);
        /*---------------ota----------------------*/
        // char ver_t[50];
        // if(Get_value(ver_t,Http_res,(char *)"$v:")> 0)
        // {
        //     sprintf(URL_REQUEST,"http://%s/Content/OTA/%s.bin",Url,ver_t); 
        //     Debug.printf("new version %s is availability\n",ver_t);
        //     Wifi_Http_ota(URL_REQUEST);
        // }
        
        if( Process_content_http(Http_res)>0)
        {
          Callrequest=1;
        }
        else
        {
            //Debug.println("No request from server");
        }
        Http_err_cnt=0;
    }
    else
    {
      
      Http_err_code[Http_err_code_cnt] = answer;
      Http_err_code_cnt++;
      #if SEND_REPORT_HTTP
        if(Http_err_code_cnt ==10)
        {
          char PhoneNum_Report_t[20];
          sprintf(PhoneNum_Report_t,"\"%s\"",PhoneNum_Report);
          Send_report_http_code(PhoneNum_Report_t);
          Http_err_code_cnt =0;
        }
      #endif
      Debug.printf("Http_err: %d ",answer);
      Http_err_cnt ++;
      if(Http_err_cnt %3 ==0)
      {
        Callrequest = 2;
      }
      if(Http_err_cnt == RST_AF_HTTP_ERR)
      {
          Debug.println("Http Erro => restart module");
          delay(10000);
          ESP.restart();
      }
      delay(2000);
    }
}
extern void __disconnet();
void Check_sim_ready(void)
{
  if(At_Command((char *)"AT",(char *)"OK\r\n",5000)<=0)
    __disconnet();
   
  if(At_Command((char *)"AT+CPIN?",(char *)"+CPIN: READY\r\n",5000)> 0)
  {
    Cpinready =1;
    return ;
  } 
  else
  {
    if(Module_type == TYPE_UC15)
      At_Command((char *)"AT+QPOWD",(char *)"OK\r\n",5000);// < 0) // quectel
    else  if(Module_type == TYPE_SIM7600CE)
      At_Command((char *)"AT+CFUN=6",(char *)"OK\r\n",5000);// < 0) // sim7600
    else  if(Module_type == TYPE_A7600C)
      At_Command((char *)"AT+CFUN=6",(char *)"OK\r\n",5000);// < 0) // A7600
    else
      At_Command((char *)"AT+CPOWD=1",(char *)"POWER DOWN",5000);       // simcom
    for(uint8_t i=10;i>0;i--)
    {
      Debug.printf("Esp will be reset affter %d s\n",i);
      if(At_Command((char *)"AT",(char *)"OK\r\n",1000)>0)
        break;
    }
    ESP.restart();
  }
}
int Find_StringNumber(char *des,char *src, int bNum,int matchtype)
{
    matchtype = matchtype;
    int i,j;
    int src_len = strlen(src);
    for (i =0;i< src_len;i++)
    {
        if((src[i] >= 0x30) && (src[i] <= 0x39)) // get only number
        {
          if((i+bNum) > src_len)
          {
            return -1;
          }
          for(j=i;j<(bNum+i);j++)
          {
            if(!((src[j] >= 0x30) && (src[j] <= 0x39)))
            {
              break;
            }
          }
          if(j == (bNum+i))
          {
            memcpy(des,&src[i],bNum);
            des[bNum]=0;
            return bNum;
          }
          else
          {
              i+=bNum;
          }
        }
    }
    return -1;
}

void Proces_Freebufer(void)
{
  if(AT_Buff_Free_cnt)
  {
    memset(AT_Buff_Free,0,AT_Buff_Free_cnt);
    AT_Buff_Free_cnt = 0;
  }
}