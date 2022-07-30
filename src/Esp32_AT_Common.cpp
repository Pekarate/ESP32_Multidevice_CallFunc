#include "Esp32_AT_Hal.h"
#include "Arduino.h"
#include "Esp32_AT_Common.h"
#include "Esp32_define.h"

uint8_t AT_Done =0;
uint8_t Cpinready =0;
/*specific type_of_moudle */

void __reset()
{
  pinMode(Reset_Pin,OUTPUT);
  pinMode(Reset_Pin_1,OUTPUT);
  
  digitalWrite(Reset_Pin,HIGH);
  digitalWrite(Reset_Pin_1,LOW);
  delay(2000);

  digitalWrite(Reset_Pin,LOW);
  digitalWrite(Reset_Pin_1,HIGH);
  delay(5000);
}

void __disconnet()
{
  uint8_t i=0;
  for( i=0;i<10;i++)
  {
    if(At_Command((char *)"AT",(char *)"OK\r\n",5000)>0)
      return;
    delay(100);
  }

  AT_Write((uint8_t *)"+++",3);
  int n = 0,res=0;
	res = At_Command((char *)"ATH",(char *)"NO CARRIER",3000);
	while (res == 0) {
		n++;
		if (n > 5) {
      if(n>15)
			  ESP.restart();
			delay(1000);
			AT_Write((uint8_t *)"+++",3);
			delay(1000);
		}
		delay(1000);
		res = At_Command((char *)"ATH",(char *)"NO CARRIER",3000);
	}
}

/*
    khoi tao cac ham co ban cho sim 8000
*/
int AT_Device_Init(int baud)
{
  #if AT_DEBUG
    Debug.println("AT_Device_Init");

  #endif  
    AT_Port_Init(baud);
    int answer;
    uint8_t i=0;
    // for( i=0;i<10;i++)
    // {
    //   if(At_Command((char *)"AT",(char *)"OK\r\n",5000)>0)
    //     break;
    //   delay(20);
    // }
    __reset();
    __disconnet();
    for( i=0;i<10;i++)
    {
      if(At_Command((char *)"ATH",(char *)"OK\r\n",5000)>0)
        break;
      delay(20);
    }
    for(uint8_t i=0;i<10;i++)
    {
      if(At_Command((char *)"ATE0",(char *)"OK\r\n",5000)>0)
        break;
      delay(20);
    }
    for(uint8_t i=0;i<10;i++)
    {
      if(At_Command((char *)"AT+CSCS=\"GSM\"", (char *)"OK\r\n", 5000)>0)
        break;
      delay(20);
    }
    uint32_t st =  millis()+60000;
    while(millis() <st)
    {
        if(At_Command((char *)"AT+CREG=2",(char *)"OK\r\n",5000)> 0)
        break;
        delay(20);
    }

    do { 
      answer = At_Command((char *)"AT+CMGF=1", (char *)"OK\r\n",1000);
    } while(answer==0);          // Set SMS into text mode

    do { 
      answer = At_Command((char *)"AT+CNMI=2,0,2,0,0", (char *)"OK\r\n",1000);
    } while(answer==0);  // Set Message mode when receive new SMS
    return 1;
}

int AT_Get_Sim_IMEI(char *Imei) 
{
  AT_Send((char *)"AT+CIMI");
  uint8_t des[101] = {0};
  if(AT_read_until(des,(char *)"OK\r\n",100,2000)>10)
  {
    if(strstr((char *)des,"OK"))
    {
      if(Find_StringNumber(Imei,(char *)des,15,0)==15)
      {
        return 15;
      }
    }
  }
  return -1;
}
int AT_Get_Module_IMEI(char *Imei) 
{
  AT_Send((char *)"AT+GSN");
  uint8_t des[101] = {0};
  if(AT_read_until(des,(char *)"OK\r\n",100,2000)>10)
  {
    if(strstr((char *)des,"OK"))
    {
      uint8_t len;
      if(Find_StringNumber(Imei,(char *)des,15,0)==15)
      {
        return 15;
      }
    }
  }
  return -1;
}
