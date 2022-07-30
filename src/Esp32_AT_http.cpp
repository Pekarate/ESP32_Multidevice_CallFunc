#include "Esp32_AT_Hal.h"
#include "Arduino.h"
#include "Esp32_AT_http.h"
#include "Esp32_define.h"

int Status_Code = 0;
int Content_length = 0;
int isHttpInit = 0;
int AT_Http_Read_Statuscode()
{
  return Status_Code;
}

int AT_Http_Read_Rontentlength()
{
  return Content_length;
}

int AT_Http_Read_Data(char *des,int len)
{
  AT_Send((char *)"AT+HTTPREAD");
  char *des_t = (char *)malloc(2*len);
  memset(des_t,0,2*len);
  int tot = AT_read_until((uint8_t *)des_t,(char *)"OK\r\n",1000,5000);
  if(tot>4)
  {
    des_t[tot] =0;
    if(strstr((char *)des_t,"+HTTPREAD:"))
    {
      char *p = strstr((des_t+2),"\r\n");
      char *end = strstr(p,"\r\nOK\r\n");
      int size  = strlen(p) - strlen(end) - 2;  ///r/n start of p
      //Debug.printf("body : %d byte",size);
      memcpy(des,p+2,size);
      des[size] =0;
      free(des_t);
      return size;
    }
  }
  free(des_t);
  return -1;
  //Debug.printf("%d : %s",tot,des_t);
  //if(strstr((char *)des_t),)

}

int AT_SIM7600_Http_Read_Data(char *des,int len)
{
  char cmd[40]={0};
  
  sprintf(cmd,"AT+HTTPREAD=0,%d",len);
  char rsp[40]={0};
  sprintf(rsp,": %d\r\n",len);
  int test = At_Command(cmd,rsp,1000);
  Debug.printf("HTTP READ CMD:%d",test);
  char des_t[100]={0};
  int tot = AT_read_until((uint8_t *)des_t,(char *)"+HTTPREAD:0",100,5000);
  //Debug.printf("-%s-",des_t);
  Debug.printf("HTTP READ BYTE:%d",tot );
  char *end;
  if((end = strstr(des_t,(char *)"\r\n+HTTPREAD")))
  {
    des_t[tot] =0;
    int s = strlen(des_t) - strlen(end);
    memcpy(des,des_t,s);
    des[s] =0;
    Debug.printf("DONE:%d : %s",tot,des_t);
    return s;
  }
  if((end = strstr(des_t,(char *)"\r\nOK")))
  {
    des_t[tot] =0;
    int s = strlen(des_t) - strlen(end);
    memcpy(des,des_t,s);
    des[s] =0;
    Debug.printf("DONE:%d : %s",tot,des_t);
    return s;
  }
  Debug.printf("FAIL: %d : %s",tot,des_t);
  return -1;
  
  //if(strstr((char *)des_t),)

}

int AT_SIM7600_HTTP_Get(char * request_url,char *rsp)
{
  static int Is_SocKet_Init = 0;
  char buf[1024];
  char *para;
  Status_Code = 0;
  if(Http_Try>3)
    Http_Try =3;
  while(Http_Try>0)
  {
    if(!Is_SocKet_Init)
    {
      At_Command((char *)"AT+CGSOCKCONT=1,\"IP\",\"CMNET\"",(char *)"OK\r\n",5000);
      At_Command((char *)"AT+CSOCKSETPN=1",(char *)"OK\r\n",5000);
      At_Command((char *)"AT+CIPMODE=0",(char *)"OK\r\n",5000);
      At_Command((char *)"AT+NETOPEN",(char *)"NETOPEN: 0",5000);

    }

    if(At_Command((char *)"AT+CIPOPEN=0,\"TCP\",\"ibsmanage.com\",80",(char *)"CIPOPEN: 0,0",5000)>0)
    {
      para = strstr(request_url,"/Active");
      int len = sprintf(buf,"GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "Cache-Control: max-age=0\r\n"
                "Upgrade-Insecure-Requests: 1\r\n"
                "User-Agent: ESP32\r\n"
                "Accept: text/html\r\n"
                "Accept-Encoding: deflate\r\n"
                "Accept-Language: q=0.9,en-US;\r\n\r\n",para,Url
              );
        #if AT_DEBUG
//        Debug.printf("write header: %d byte: ---------%s------\n",len,buf);
        #endif
        char cmd[100];
        sprintf(cmd,"AT+CIPSEND=0,%d",len);
        At_Command_Without_Endline((char *)cmd,(char *)">",5000);
        AT_Write((uint8_t *)buf,len);
        uint8_t aux_string=26;
        AT_Write(&aux_string,1);              // Ctrl + Z
        #if AT_DEBUG
        Debug.println("AT_Write 26");
        #endif
        AT_read_until((uint8_t *)buf,(char *)"CLOSE:",1024,30000);
        if(strstr(buf,"CLOSE:"))
        {
          int sizess =0;
          Is_SocKet_Init =1;
          Status_Code = 200;
          char *start,*end;
          if((start = strstr(buf+30,"\r\n\r\n"))) //bo qua header
          {
            if((end = strstr(start+4,"\r\n")))
            {
              sizess = (int)end - int(start)-4;
              if(sizess>0)
              {
                memcpy(rsp,start +4,sizess);
                rsp[sizess] =0;
              }
              
              #if AT_DEBUG
                Debug.printf("reqeuest successful %d byte:%s\n",sizess,rsp);
              #endif
              AT_Free_rx_buffer();
              return 200;
            }
          }
        }
        Debug.printf("request fail buf: %s",buf);
        
    }
    else{
      if(At_Command((char *)"AT+CIPCLOSE=0",(char *)"CIPCLOSE: 0,0",3000) <=0)
      {
          At_Command((char *)"AT+NETCLOSE",(char *)"OK\r\n",5000);
          Is_SocKet_Init =0;
      }
      delay(500);
    }
    Http_Try --;
  }
  At_Command((char *)"AT+NETCLOSE",(char *)"OK\r\n",5000);
  Is_SocKet_Init =0;
  return 1;
}

int AT_Http_Request(char * request_url,char *rsp)
{
 // int res = 0;
  int tmp =0;
  static int Is_Http_Init = 0;
  Status_Code = 0;
  Content_length = 0;
  //At_Command((char *)"AT+HTTPSTATUS?",(char *)"OK\r\n",5000);
  if(!Is_Http_Init)
  {
    if( Module_type != TYPE_A7600C)
    {
      if(At_Command((char *)"AT+SAPBR=2,1",(char *)"SAPBR: 1,1",10000)<0)
      {
        At_Command((char *)"AT+CGATT=1",(char *)"OK\r\n",2000);
        At_Command((char *)"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",(char *)"OK",2000);
        At_Command((char *)"AT+SAPBR=3,1,\"APN\",\"CMNET\"",(char *)"OK",5000);
        At_Command((char *)"AT+SAPBR=1,1",(char *)"OK\r\n",10000);
      }
    }
    At_Command((char *)"AT+HTTPINIT",(char *)"OK\r\n",5000);
    At_Command((char *)"AT+HTTPPARA=\"CID\",1",(char *)"OK\r\n",5000);
    At_Command((char *)"AT+HTTPPARA=\"TIMEOUT\",60",(char *)"OK\r\n",5000);
  }
  char Buff[256];
  sprintf(Buff,"AT+HTTPPARA=\"URL\",%s",request_url) ;
  Debug.printf(request_url);
  Debug.printf("\n");
  At_Command(Buff,(char *)"OK",10000);
  if(At_Command((char *)"AT+HTTPACTION=0",(char *)"OK\r\n",10000)<0)//submit the GET request
    return -1;
  int len =0;
  //Debug.printf("read result start\n");
  uint8_t cnt =5;
  while(cnt --)
  {
    memset(Buff,0,256);
    len = AT_read_until((uint8_t *)Buff,(char *)"\r\n",255,15000);
    if(strstr(Buff,"HTTPACTION"))
    { 
      cnt = 10;
       break;
    }
  }
  if(cnt == 10)
  {
    Buff[len]=0;
    Debug.printf(" result :%s\n",Buff);
    if((tmp = AT_Getint_index(&Status_Code,Buff,(char *)": ",1))>0)
    {
      if(AT_Getint_index(&Content_length,Buff,(char *)": ",2)>0)
      {
        
        if(Status_Code ==200)
        {
          Is_Http_Init = 1;
          int r;
          if( Module_type = TYPE_A7600C)
          {
            r= AT_SIM7600_Http_Read_Data(rsp,Content_length);
          }
          else{
            r= AT_Http_Read_Data(rsp,100);
          }
           
          Debug.printf("Content_length: %d,Http_read_data: %d\r\n",Content_length,r);
          if(r == Content_length)
          {
            Debug.println("Get data done");
          }
          return Status_Code;
        }
      }
      else
      {
        Debug.printf("Http request error: %d %d\r\n",Status_Code,Content_length);
        if(Status_Code == 604)  //looi 604
        {
          Is_Http_Init =0;
          At_Command((char *)"AT+HTTPSTATUS?",(char *)"OK\r\n",5000);
          while (1)
          {
            if(At_Command((char *)"AT+HTTPTERM",(char *)"OK\r\n",5000)>0)
              return Status_Code;
            delay(500);
          }    
        }
      }
    }
  }
  else
  {
    #if AT_DEBUG
      Debug.printf("read result timeout\n");
    #endif
  }
  At_Command((char *)"AT+HTTPTERM",(char *)"OK\r\n",5000);
  Is_Http_Init =0;
  return Status_Code;
}
int AT_UC15_Http_Read_Data(char *des,int len)
{
  char des_t[100];
  char des_t1[20]={0};
  memset(des_t,0,100);
  At_Command((char *)"AT+QHTTPREAD=1",(char *)"CONNECT\r\n",1000);
  int tot = AT_read_until((uint8_t *)des_t,(char *)"OK\r\n",1000,1000);
  AT_read_until((uint8_t *)des_t1,(char *)"\r\n",20,500); //read result http_read
  if(tot>4)
  {
    memcpy(des,des_t,tot-4); //-OK/r/n
    des[tot-4]=0;
    #if AT_DEBUG
    Debug.printf("=>%s\n",des);
    #endif
    return tot-4;
  }
  return -1;
}
uint8_t set_responseheader =0;
uint8_t QICSGP =0;
int AT_UC15_HTTP_Get(char* url,char *res)
{
  Status_Code = 0;
  Content_length = 0;
  // set PDP ID 
  At_Command((char *)"AT+QHTTPCFG=\"contextid\",1",(char *)"OK\r\n",2000);
  if(set_responseheader ==0)
  {
    At_Command((char *)"AT+QHTTPCFG=\"responseheader\",0",(char *)"OK\r\n",2000);
    set_responseheader =1;
  }
  if(QICSGP ==0)
  {
    At_Command((char *)"AT+QICSGP=1,1,\"INTERNET\",\"\",\"\",1",(char *)"OK\r\n",2000);
    QICSGP =1;
  }
  At_Command((char *)"AT+QIACT?",(char *)"OK\r\n",5000);
  if(!strstr(AT_Buff,"1,1,1"))
    At_Command((char *)"AT+QIACT=1",(char *)"OK\r\n",5000);
  int  Url_len  = strlen(url);
  char Buff[50];
  #if AT_DEBUG
  Debug.printf("Get: %s\n",URL_REQUEST);
  #endif
  sprintf(Buff,"AT+QHTTPURL=%d,10",Url_len);
  At_Command(Buff,(char *)"CONNECT\r\n",1000);
  AT_Write((uint8_t *)url,Url_len);
  memset(Buff,0,50);
  AT_read_until((uint8_t *)Buff,(char *)"OK\r\n",20,5000);
  At_Command((char *)"AT+QHTTPGET=80",(char *)"OK\r\n",2000);
  memset(Buff,0,50);
  int len = AT_read_until((uint8_t *)Buff,(char *)"\r\n",100,15000);
  int tmp;
  if(len>0)
  {
    Buff[len]=0;
    #if AT_DEBUG
    Debug.printf(" result :%s\n",Buff);
    #endif
    if((tmp = AT_Getint_index(&Status_Code,Buff,(char *)": ",1))>0)
    {
      if(AT_Getint_index(&Content_length,Buff,(char *)": ",2)>0)
      {
        if(Status_Code ==200)
        {
          int r = AT_UC15_Http_Read_Data(res,100);
          #if AT_DEBUG
          //Debug.printf("Content_length: %d,Http_read_data: %d\r\n",Content_length,r);
          #endif
          if(r == Content_length)
          {
            #if AT_DEBUG
            //Debug.println("Get data done");
            #endif
          }
        }
      }
      else
      {
        #if AT_DEBUG
        Debug.printf("Http request error: %d %d\r\n",Status_Code,Content_length);
        #endif
      }
    }
  }
  return Status_Code;
}

int HTTP_POST(char * request_url,char *rsp)
{
  

  if(At_Command((char *)"AT+SAPBR=2,1",(char *)"SAPBR: 1,1",10000)<0)
  {
    At_Command((char *)"AT+CGATT=1",(char *)"OK\r\n",2000);
    At_Command((char *)"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",(char *)"OK",2000);
    At_Command((char *)"AT+SAPBR=3,1,\"APN\",\"\"",(char *)"OK",5000);
    At_Command((char *)"AT+SAPBR=1,1",(char *)"OK\r\n",10000);
  }
  At_Command((char *)"AT+HTTPINIT",(char *)"OK\r\n",5000);
  char Url[1024];
  sprintf(Url,"AT+HTTPPARA=\"URL\",%s",request_url) ;
  At_Command(Url,(char *)"OK",10000);

  At_Command((char *)"AT+HTTPPARA=\"CONTENT\",application/json",(char *)"OK",10000);

  At_Command((char *)"AT+HTTPPARA=\"USERDATA\",APIKEY: 06E39A14738939A649CEAF93D8B93204",(char *)"OK",10000);

  int len = sprintf(Url,"{\"Station\": \"001\"}");
  char cmd[100];
  sprintf(cmd,"AT+HTTPDATA=%d,1000",len);
  At_Command(cmd,(char *)"DOWNLOAD",10000);
  AT_Write((uint8_t *)Url,len);
  uint8_t des[1024]={0};
  AT_read_until(des,(char *)"OK",1024,2000);
  // request_result = "";
  // mySerial.println();
  if(At_Command((char *)"AT+HTTPACTION=1",(char *)"OK\r\n",3000)<0)//submit the GET request
    return -1;
  len =0;
  char Buff[1024]={0};
  int tmp;
  Debug.printf("read result start\n");
  len = AT_read_until((uint8_t *)Buff,(char *)"\r\n",100,15000);
  if(len>0)
  {
    Buff[len]=0;
    Debug.printf(" result :%s\n",Buff);
    if((tmp = AT_Getint_index(&Status_Code,Buff,(char *)": ",1))>0)
    {
      if(AT_Getint_index(&Content_length,Buff,(char *)": ",2)>0)
      {
        if(Status_Code ==200)
        {
          int r = AT_Http_Read_Data(rsp,100);
          //Debug.printf("Content_length: %d,Http_read_data: %d\r\n",Content_length,r);
          if(r == Content_length)
          {
            //Debug.println("Get data done");
          }
        }
      }
      else
      {
        Debug.printf("Http request error: %d %d\r\n",Status_Code,Content_length);
        if(Status_Code == 604)  //looi 604
        {
          //At_Command((char *)"AT+HTTPSTATUS?",(char *)"OK\r\n",5000);
          while (1)
          {
            if(At_Command((char *)"AT+HTTPTERM",(char *)"OK\r\n",5000)>0)
              return Status_Code;
            delay(500);
          }    
        }
      }
    }
  }
  else
  {
    #if AT_DEBUG
    Debug.printf("read result timeout\n");
    #endif
  }
  At_Command((char *)"AT+HTTPTERM",(char *)"OK\r\n",5000);
  return Status_Code;
}
