#include "Arduino.h"
#include "Esp32_define.h"
#include "Esp32_AT_sms_call.h"
#include "Esp32_AT_Hal.h"
#define BUFF_SIZE 1024
char Buff[BUFF_SIZE];

extern int Sms_Process(char *info,char *content);

int AT_Call_SetAutoAnswer(void)
{
  return At_Command((char *)"ATS0=000", (char *)"OK\r\n", 5000); //Automatic answering mode is disable
}

int AT_Call_To(char* input_number)
{
  char aux_string[50];
  #if AT_DEBUG
  Debug.printf("Calling: %s\r\n",input_number);
  #endif
  sprintf(aux_string,"ATD%s;", input_number);
  return At_Command(aux_string,(char *)"OK\r\n",5000);     // Send the SMS number
}
int AT_call_Waitresult(int Calltime)
{
  uint32_t st = millis();
  uint8_t Check_call_exception =0;
  while (1)
  {
    memset(Buff,0,BUFF_SIZE);
    if(AT_read_until((uint8_t *)Buff,(char *)"\r\n",256,1000)>0)
    {
      if(strstr(Buff,"NO CARRIER")) //ket thuc cuoc goi
      {
        Debug.printf("call done: %lu ms\n",millis() - st);
        return (millis() - st);
      }
      else if(strstr(Buff,"NO ANSWER") ||strstr(Buff,"BUSY"))
      {
        Debug.printf("CALL IS NO ANSWER OR BUSY\n");
        return 0;
        /* code */
      }
      else
      {
        /* code */
        Debug.printf("Call status not found\n");
        return 0;
      }
    }
    AT_Sms_Getlist();
    if(New_Otp) //end call if send OTP fail
    {
      break;
    }
    if(millis() - st > Calltime*1000)
      break;
    if((Check_call_exception++) > 5)
    {
      Check_call_exception = 0;
      if(At_Command_nodebug((char *)"AT+CPAS",(char *)"OK\r\n",2000))
      {
        if(strstr(AT_Buff,(char *)"CPAS: 0") || strstr(AT_Buff,(char *)"CPAS: 1")||strstr(AT_Buff,(char *)"CPAS: 2"))
        {
          printf("return without endcall detected\r\n");
          return millis()-st;
        }
      }
    }
  }
  Debug.println("cancel call");
  uint8_t cnt =5;
  while(cnt --)
  {
    if(AT_Call_Hangup()>0)
      break;
  }
  if(cnt != 0)
  {
    Debug.printf("call done: %lu ms\n",millis() - st);
  }
  else
      Debug.printf("ATH err ,request reset module sim");
  return (millis() - st);
}

int AT_SIM7600_call_Waitresult(int Calltime)
{
  uint32_t timecallstart =millis();
  uint8_t callbegin=0;
  memset(Buff,0,BUFF_SIZE);
  uint32_t tt = Calltime*1000;
  char tm[10];
  char *start;
  char *stop;
  uint8_t Check_call_exception =0;
  while((millis() - st <Calltime*1000*3))
  {
    memset(Buff,0,BUFF_SIZE);
    if((Check_call_exception++) > 100)
    {
      Check_call_exception = 0;
      if(At_Command_nodebug((char *)"AT+CPAS",(char *)"OK\r\n",2000))
      {
        if(strstr(AT_Buff,(char *)"CPAS: 0") || strstr(AT_Buff,(char *)"CPAS: 1")||strstr(AT_Buff,(char *)"CPAS: 2"))
        {
          printf("return without endcall detected\r\n");
          return millis()-timecallstart;
        }
      }
    }
    if(AT_read_until((uint8_t *)Buff,(char *)"\r\n",256,500)>0)
    {
      #if AT_DEBUG
        Debug.printf("result: %s\n",Buff);
      #endif
      if(strstr(Buff,"NO CARRIER")) //ket thuc cuoc goi
      {
        if(callbegin)
        {
          #if AT_DEBUG
          Debug.printf("call done: %lu ms\n",millis() - st);
          #endif
          return (millis() - timecallstart);
        }
        else
        {
          #if AT_DEBUG
           Debug.printf("CALL IS NO ANSWER OR BUSY\n");
          #endif
          return 0;
        }
      }
      else if(strstr(Buff,"NO ANSWER") ||strstr(Buff,"BUSY"))
      {
        #if AT_DEBUG
        Debug.printf("CALL IS NO ANSWER OR BUSY\n");
        #endif
        return 0;
        /* code */
      }
      else if(strstr(Buff,"VOICE CALL: BEGIN"))
      {
        #if AT_DEBUG
          Debug.printf("CALL START \n");
        #endif
        callbegin =1;
        timecallstart = millis();
        /* code */
      }
      else if(strstr(Buff,"VOICE CALL: END"))
      {
        #if AT_DEBUG
          Debug.printf("CALL END \n");
        #endif
        char tm[10];
        start = strstr(Buff,"END:");
        stop = strstr(start,"\r\n");
        if((start) && (stop))
        {
          int s = strlen(start) - strlen(stop)-4;
          if(s >0)
          {

            memcpy(tm,start+4,s);
            tm[s]=0;
            #if AT_DEBUG
              Debug.printf("time got : %s\n",tm);
            #endif
            s = atoi(tm);
            return ((s/100 *60) +(s%100))*1000;
          }
        }
        #if AT_DEBUG
              Debug.printf("get time fail\n");
        #endif
        return millis()-timecallstart;
        /* code */
      }
      else
      {
        /* code */
        #if AT_DEBUG
        Debug.printf("Call status not found: %s \n",Buff);
        #endif
      }
    }

    AT_Sms_Getlist();
    if(New_Otp)   //end call if send OTP fail
    {
    #if AT_DEBUG
        Debug.printf("have new OTP => end\n");
    #endif
      break;
    }
    
    if(callbegin)
    {
      if(millis() -timecallstart>tt )
      {
        #if AT_DEBUG
          Debug.printf("time end\n");
        #endif
          break;
      }
    }
  }
  At_Command((char *)"AT+CHUP",(char *)"OK\r\n",2000);
  start = strstr(AT_Buff,"END:");
  stop = strstr(start,"\r\n");
  if((start) && (stop))
  {
    int s = strlen(start) - strlen(stop)-4;
    if(s >0)
    {

      memcpy(tm,start+4,s);
      tm[s]=0;
      #if AT_DEBUG
        s = atoi(tm);
        Debug.printf("time got : %s <=> %ds\n",tm,s);
      #endif
      
      return ((s/100 *60) +(s%100))*1000;
    }
  }
  #if AT_DEBUG
        Debug.printf("get time fail\n");
  #endif
  return Calltime*1000;
}


int AT_Call_Hangup(void)
{
  if(At_Command((char *)"AT+CHUP",(char *)"OK\r\n",3000)>0)   
    return 1;
  if(At_Command((char *)"ATH",(char *)"OK\r\n",3000)>0)   
    return 1;
  return 0;
}
void AT_Sms_Delele(int index,int flag)
{
  sprintf(Buff,"AT+CMGD=%d,%d",index,flag);
  int res = At_Command(Buff, (char *)"OK\r\n", 2000); 
  if(res == 1 ) {
    #if AT_DEBUG
    Debug.println("SMS deleted");
    #endif
  } else {
    #if AT_DEBUG
    Debug.println("SMS delete fail");
    #endif
  }
}
int AT_Sms_Decoder(char *sms)
{
  char *p = NULL;
  p = strstr(sms,"+CMGL: ");
  if(p==NULL)
    return 0;
  char *p_t,*p_t_t;
  char info[100];
  char content[512];
  int size =0,cnt =0;
  while(p!=NULL)
  {
    p_t = strstr(p,"\r\n");
    size = (int)p_t - (int)p;
    if(size <0)
      return -1;
    memcpy(info,p,size);
    info[size]=0;
    p_t_t = strstr(p_t+2,"+CMGL");
    if(p_t_t ==NULL)
    //printf("p_t_t: %d p_t: %d\n",(int)p_t_t,(int)p_t);
    {
      p_t_t = strstr(p_t+2,"OK");
    }
    if(p_t_t!= NULL )
    {
      size = (int)p_t_t - (int)p_t-2;
      if(size <0)
        return -1;
      memcpy(content,p_t+2,size);
      content[size]=0;
    }
    #if AT_DEBUG
    Debug.printf("Info: %s\n",info);
    Debug.printf("Content: %s\n",content);
    #endif
    Sms_Process(info,content);
    p = strstr(p+4,"CMGL: ");
    cnt++;
  }
  return cnt;
}
int AT_Sms_Getlist()
{
  static int Set_Text_Mode =11;
  char data[1025] = {0};
  if(Set_Text_Mode>10)
  {
    if(At_Command_nodebug((char *)"AT+CMGF=1", (char *)"OK\r\n",1000) > 0) // reset command affter 10 times
    {
      Set_Text_Mode = 0; 
    }
  }
  Set_Text_Mode++;
  //At_Command((char *)"AT+CMGF=1", (char *)"OK\r\n",1000);
  AT_Send((char *)"AT+CMGL=\"ALL\"");
  int tot =AT_read_until((uint8_t *)data,(char *)"OK\r\n",1024,2000);
  //Debug.printf("Message read: -------------\n%s\n----------------",data);
  if((!strstr(data,(char *)"OK\r\n")) && (!strstr(data,(char *)"ERROR"))) ///co cuoc goi toi
  {
    //Debug.printf("Message read continue..\n");
    //read until OK/r/n
    char tmp[257];
    int s = AT_read_until((uint8_t *)tmp,(char *)"OK/r/n",256,2000);
    unsigned long st = millis() +10000;
    while(millis() < st)
    {
        if(strstr(tmp,"OK\r\n"))
        {  
          //Debug.printf("Message read done\n");
          break;
        }
        else
        {
          if(s >4)
          {
            tmp[0] = tmp[s-4];
            tmp[1] = tmp[s-3];
            tmp[2] = tmp[s-2];
            tmp[3] = tmp[s-1];
            s = AT_read_until((uint8_t *)tmp+4,(char *)"OK/r/n",250,2000);
          }
          else
            s = AT_read_until((uint8_t *)tmp+s,(char *)"OK/r/n",250,2000);
        }
    }
  }
  else
    //Debug.printf("Message read done\n");
  //Debug.printf("AT_Getlist_sms: %s",data);
  if(strstr(data,(char *)"RING\r\n")) ///co cuoc goi toi
    return 0;
  if(strstr(data,(char *)"ERROR\r\n")) ///get sms error
     Check_sim_ready();
  if(strstr(data,"+CMGL"))
  {
    if((strstr(data,"status?")!=0) || (strstr(data,"server:")!=0)|| (strstr(data,"p:")!=0) || (strstr(data,"wifi:")!=0)
     || (strstr(data,"sc:")!=0) || (strstr(data,"sdt:")!=0)|| (strstr(data,"mode:")!=0) || (strstr(data," OTP")!=0))
    {
      tot = AT_Sms_Decoder((char *)data);
      if(tot <0)
      {
        //Debug.printf("Error when process sms;");
        AT_Sms_DeleleAll();
      }
    }
    else
    {
      //Debug.printf("There are no valid messages\n");
      AT_Sms_DeleleAll();
    }
  }
  else 
  {
    Debug.printf("#");
    tot =0;
  }
  return tot;
}
void AT_Sms_DeleleAll(void)
{
  AT_Sms_Delele(1,4);
}


int AT_Sms_Send(char* input_number,char* msg)
{
  char aux_string[256] = {0};
  sprintf(aux_string,"AT+CMGS=%s", input_number);
  At_Command(aux_string,(char *)">",5000);     // Send the SMS number
  AT_Write((uint8_t *)msg,strlen(msg));    
  #if AT_DEBUG
  Debug.println("AT_Write msg");
  #endif
  aux_string[0]=26;
  AT_Write((uint8_t *)aux_string,1);              // Ctrl + Z
  #if AT_DEBUG
  Debug.println("AT_Write 26");
  #endif
  int t = AT_read_until((uint8_t *)aux_string,(char *)"OK\r\n",1000,5000);
  if(t > 0 )
  {
    aux_string[t]=0;
    #if AT_DEBUG
    Debug.printf("smms: %s",aux_string);
    #endif
    if(strstr(aux_string,"OK\r\n"))
      return 1;
  }
  return -1;
} 
