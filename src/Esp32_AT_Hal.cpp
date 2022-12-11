#include <Arduino.h>
#include "Esp32_AT_Hal.h"
#include "stdlib.h"
#include "Esp32_define.h"

#define AT_Port Serial2


#define MODEM_RX 16
#define MODEM_TX 17

#define RST_SIM 4
#define RST_SIM_ON() digitalWrite(RST_SIM, LOW)
#define RST_SIM_OFF() digitalWrite(RST_SIM, HIGH)

char AT_Buff[1024];
char AT_Buff_Free[1025] = {0};
uint16_t AT_Buff_Free_cnt = 0;
 int AT_Getstring_index(char *des,char *scr,char *key,int index);
int AT_Getint_index(int *res,char *src,char *key,int index);
void AT_Free_rx_buffer(void)
{
  #if AT_DEBUG
  uint8_t bufer_updated =0;
  #endif
  while( AT_Port.available() > 0) {
    #if AT_DEBUG
      bufer_updated =1;
    #endif
    AT_Buff_Free[AT_Buff_Free_cnt] = AT_Port.read();
    AT_Buff_Free_cnt++;
    if(AT_Buff_Free_cnt >1023)
    {
      AT_Buff_Free_cnt =0;
    }
  }
  #if AT_DEBUG
    if(bufer_updated)
    {
      Debug.printf("\n<FREE:%s>\n",AT_Buff_Free);
      bufer_updated =0;
    }    
  #endif
}
void AT_Send(char *data)
{
  AT_Free_rx_buffer();
  AT_Port.println(data);
  AT_Port.flush();
}
void AT_Write(uint8_t *data,uint16_t len)
{
  AT_Free_rx_buffer();
  for(uint16_t i=0;i<len;i++)
  {
    AT_Port.write(data[i]);
  }
  AT_Port.flush();
}
/*
  read data in rx data in timeout ms 
  return byte total received
*/
int AT_read_until(uint8_t *Des,char *end,uint16_t len,uint32_t timeout)
{
    uint32_t st = millis()+timeout;
    uint16_t tot =0;
    while(millis() < st)
    {   
        if(AT_Port.available() != 0){    
            Des[tot] = AT_Port.read();
            tot++;
            if(tot == len)
                break;
            if(strstr((char*)Des+2,end))
            {
              break;
            }
            if(strstr((char*)Des+1,(char *)"RING")) 
            {
              #if AT_DEBUG
              Debug.println("call in coming");
              #endif
              call_incoming = 1;
              //break;

            }
            if(tot>5) 
            {
              if((Des[tot-1] == '\n')&&(Des[tot-2] == '\r'))
              {
                AT_Check_unexpected_Response((char *)Des,&tot);
              }
            }
        }
        #if ESP32
          timerWrite(timer, 0);  
        #endif
    }
    Des[tot]=0;
    #if AT_DEBUG
     //Debug.printf("AT recv %d byte %d\n",tot,(int)(millis() - st));
    #endif
    AT_Check_unexpected_Response((char *)Des,&tot);
    return tot;
}
/*
  not wait return /r/n
*/
int At_Command_Without_Endline(char *cmd ,char *RSP1,uint32_t timeout)
{
  if(call_incoming)
    return -1;
  int res = -1;
  
  AT_Send(cmd);
  #if AT_DEBUG
    Debug.printf("CMD: %s\n",cmd);
    Debug.flush();
    uint32_t st1 = millis();
  #endif
  memset(AT_Buff,0,sizeof(AT_Buff));
  uint32_t st = millis()+timeout;
  uint16_t tot =0;
  while (millis() < st)
  {   
      if(AT_Port.available() != 0){    
          AT_Buff[tot] = AT_Port.read();
          tot++;
          if(strstr(AT_Buff+1,RSP1))
          { 
            res = 1;
            break;
          }
          else
          {
            if(strstr(AT_Buff,(char *)"ERROR"))
            {
                res = ERROR;
                break;
            }
          }
          if(strstr((char*)AT_Buff,(char *)"RING")) 
          {
            #if AT_DEBUG
            Debug.println("call in coming");
            Debug.flush();
            #endif
            call_incoming = 1;
            //break;
          }   
      }
      #if ESP32
        timerWrite(timer, 0);  
      #endif
  }
  AT_Buff[tot] =0;
  #if AT_DEBUG
  char AT_Buff_t[256];
  memcpy(AT_Buff_t,AT_Buff,strlen(AT_Buff));
  AT_Buff_t[strlen(AT_Buff)]=0;
  for(uint16_t i=2;i<tot;i++)
  {
    if(AT_Buff_t[i] == '\r')
    {
      AT_Buff_t[i] = 'r';
    }
    else if(AT_Buff_t[i] == '\n')
    {
      AT_Buff_t[i] ='n';
    }
    
  }
  
  if(res == 1)
  {  
    Debug.printf("RSP OK %d byte %lu: %s \n",tot,millis() - st1,AT_Buff_t); //+2 remove /r/n
  }
  else
  {
    Debug.printf("BAD RSP %d byte %lu ms: %s \n",tot,millis() - st1,AT_Buff_t);
  }
  #endif
  AT_Check_unexpected_Response(AT_Buff,&tot);
  return res;
}
int At_Command(char *cmd ,char *RSP1,uint32_t timeout)
{
  if(call_incoming)
    return -1;
  int res = -1;
  
  AT_Send(cmd);
  #if AT_DEBUG
    Debug.printf("CMD: %s\n",cmd);
    Debug.flush();
    uint32_t st1 = millis();
  #endif
  memset(AT_Buff,0,sizeof(AT_Buff));
  uint32_t st = millis()+timeout;
  uint16_t tot =0;
  while (millis() < st)
  {   
      if(AT_Port.available() != 0){    
          AT_Buff[tot] = AT_Port.read();
          tot++;
          if(strstr(AT_Buff+1,RSP1))
          { 
            res = 1;
            if(strstr(AT_Buff+2,(char *)"\r\n")) // read until /r/n
            {
              //remove response not unexpected;
               break;
            }     
          }
          else
          {
            if(strstr(AT_Buff,(char *)"ERROR"))
            {
                res = ERROR;
                if(strstr(AT_Buff+2,(char *)"\r\n")) // read until /r/n
                  break;
            }
          }
          if(strstr((char*)AT_Buff,(char *)"RING")) 
          {
            #if AT_DEBUG
            Debug.println("call in coming");
            Debug.flush();
            #endif
            call_incoming = 1;
            //break;
          }   
      }
      #if ESP32
        timerWrite(timer, 0);  
      #endif
  }
  AT_Buff[tot] =0;
  char AT_Buff_t[256];
  memcpy(AT_Buff_t,AT_Buff,strlen(AT_Buff));
  AT_Buff_t[strlen(AT_Buff)]=0;
  for(uint16_t i=2;i<tot;i++)
  {
    if(AT_Buff_t[i] == '\r')
    {
      AT_Buff_t[i] = 'r';
    }
    else if(AT_Buff_t[i] == '\n')
    {
      AT_Buff_t[i] ='n';
    }
    
  }
  #if AT_DEBUG
  if(res == 1)
  {  
    Debug.printf("RSP OK %d byte %lu: %s \n",tot,millis() - st1,AT_Buff_t); //+2 remove /r/n
  }
  else
  {
    Debug.printf("BAD RSP %d byte %lu ms: %s \n",tot,millis() - st1,AT_Buff_t);
  }
  #endif
  AT_Check_unexpected_Response(AT_Buff,&tot);
  return res;
}
int At_Command1(char *cmd ,char *RSP1,uint32_t timeout)
{
  if(call_incoming)
    return -1;
  int res = -1;
  
  AT_Send(cmd);
  #if AT_DEBUG
    Debug.printf("CMD: %s\n",cmd);
    Debug.flush();
    uint32_t st1 = millis();
  #endif
  memset(AT_Buff,0,sizeof(AT_Buff));
  uint32_t st = millis()+timeout;
  uint16_t tot =0;
  while (millis() < st)
  {   
      if(AT_Port.available() != 0){    
          AT_Buff[tot] = AT_Port.read();
          tot++;
          if(strstr(AT_Buff+1,RSP1))
          { 
            res = 1;
            if(strstr(AT_Buff+2,(char *)"\r\n")) // read until /r/n
                  break;
          }
          else
          {
            if(strstr(AT_Buff,(char *)"ERROR"))
            {
                res = ERROR;
                if(strstr(AT_Buff+2,(char *)"\r\n")) // read until /r/n
                  break;
            }
          }
          if(strstr((char*)AT_Buff,(char *)"RING")) 
          {
            #if AT_DEBUG
            Debug.println("call in coming");
            Debug.flush();
            #endif
            call_incoming = 1;
            //break;
          }   
      }
      #if ESP32
        timerWrite(timer, 0);  
      #endif
  }
  AT_Buff[tot] =0;
  char AT_Buff_t[256];
  memcpy(AT_Buff_t,AT_Buff,strlen(AT_Buff));
  AT_Buff_t[strlen(AT_Buff)]=0;
  for(uint16_t i=2;i<tot;i++)
  {
    if(AT_Buff_t[i] == '\r')
    {
      AT_Buff_t[i] = 'r';
    }
    else if(AT_Buff_t[i] == '\n')
    {
      AT_Buff_t[i] ='n';
    }
    
  }
  #if AT_DEBUG
  if(res == 1)
  {  
    Debug.printf("RSP OK %d byte %lu: %s \n",tot,millis() - st1,AT_Buff_t+2); //+2 remove /r/n
  }

  #endif
  AT_Check_unexpected_Response(AT_Buff,&tot);
  return res;
}
int At_Command_nodebug(char *cmd ,char *RSP1,uint32_t timeout)
{
  call_incoming =0;
  int res = -1;
  AT_Send(cmd);
  memset(AT_Buff,0,sizeof(AT_Buff));
  uint32_t st = millis()+timeout;
  uint16_t tot =0;
  while (millis() < st)
  {   
      if(AT_Port.available() != 0){    
          AT_Buff[tot] = AT_Port.read();
          tot++;
            if(strstr(AT_Buff+1,RSP1))
            { 
              res = 1;
              break;
            }
            else
            {
              if(strstr(AT_Buff,(char *)"ERROR"))
              {
                  res = ERROR;
                  break;
              }
            } 
      }
      #if ESP32
        timerWrite(timer, 0);  
      #endif
  }
  AT_Buff[tot] =0;
  AT_Check_unexpected_Response(AT_Buff,&tot);
  return res;
}


void AT_Port_Init(int Baud)
{
  AT_Port.begin(Baud);
}

int AT_GetInfo(char *info)
{
    if(At_Command((char *)"ATI",(char *)"OK\r\n",5000)>0)
    {
      char *p = strstr(AT_Buff+2,"OK\r\n");
      if(p)
      {
        int size = (int)p -(int)AT_Buff -2;
        if(size >0)
        {
          memcpy(info,AT_Buff+2,size);
          info[size] = 0;
          return 1;
        }
      }
    }
  return 0;
}


int AT_Getstring_index(char *des,char *scr,char *key,int index)
{
   char *p;
   if(!(p = strstr(scr,key))) //tim kiem key
      return -1;
   uint16_t len = strlen(p);
   char *tmp = (char *)malloc(len);
   len = sprintf(tmp,",%s",p+2);
   uint8_t cnt=0;
   uint16_t i=0;
   //printf("%s\n",tmp);
   for(i=0;i<len;i++)
   {
      if(tmp[i] == ',')
      {
         if(cnt == index)
            break;
         cnt++;
      }
   }
   char *start= (tmp+i+1);
   if(i != len)
   {
      //printf("find match\n");
      uint8_t tot;
      if(!(p = strstr(start,",")))
      {
         if(!(p = strstr(start,"\r")))
         {
            tot = strlen(start);
         }
         else tot = strlen(start)-strlen(p);
      }
      else
        tot = strlen(start)-strlen(p);
      memcpy(des,start,tot);
      des[tot]=0;
      free(tmp);
      return 1;
   }
   free(tmp);
   printf("not find index\n" );
   return -3;
}
int AT_Getint_index(int *res,char *src,char *key,int index)
{
   char des[20];
   if(AT_Getstring_index(des,src,key,index)<0)
   {
      return-1;
   }
   *res = atoi(des);
   return *res;
}


char ListUnexpected[NUM_UNEXPECTED][6] = {"+CREG","+CMTI"};
uint8_t CountUnexpected[NUM_UNEXPECTED] = {0};
void AT_Check_unexpected_Response(char *Bufer,uint16_t *Buf_size_control)
{
    
   char *st,*end;
   for(int cnt=0;cnt<NUM_UNEXPECTED;cnt++)
   {
      
       if((st = strstr(Bufer,ListUnexpected[cnt])))
       {
          CountUnexpected[cnt]++;
          printf("check : %s\n",ListUnexpected[cnt]);
          end = strstr(st,"\r\n");
          if (end)
          { 
            end+=2;//remove \r\n
            uint32_t byte_move_count = Bufer + *Buf_size_control - end;
            for( int i=0;i<byte_move_count;i++)
            {
              *(st+i) = *(end+i);
            }
            *Buf_size_control =*Buf_size_control - (end-st);
          }
          else
          {
            *Buf_size_control = st -Bufer;
          }
          Bufer[*Buf_size_control] = 0;
       }
   }
}
