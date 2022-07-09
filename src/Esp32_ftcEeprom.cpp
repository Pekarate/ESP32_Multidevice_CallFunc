#include "Esp32_ftcEeprom.h"
#include "string.h"
#include "Esp32_define.h"

void Eeprom_Init()
{
    EEPROM.begin(EEPROM_SIZE);
}

static void Write_to_eeprom(uint16_t addr_start,char *data,uint8_t len)
{
    int i=0;
    for ( i = 0; i < len; i++)
    {
        EEPROM.write( addr_start+i, data[i]);
        EEPROM.write( addr_start+i+1, 255);
        //Debug.printf("addr: %d,data: %c\n",SSID_ADDR_START+i,qsid[i]);
    }
    EEPROM.commit();
}


static int Read_Form_eeprom(uint16_t addr_start,char *data,uint8_t len)
{
    int i;
    for (i = 0; i < len; i++)
	{
        if ((data[i]=EEPROM.read(i+addr_start)) == 255)
            break;
        
	}
    return i;
}

// static void Write_int_to_eeprom(int addr,uint32_t data)
// {
//     // EEPROM.write( addr_start, data);
//     // EEPROM.write( addr_start+1, data>>8);
//     // EEPROM.write( addr_start+2, data>>16);
//     // EEPROM.write( addr_start+3, data>>24);
//     EEPROM.writeUInt(addr,data);
//     EEPROM.commit();
// }
// static int Read_int_to_eeprom(uint16_t addr,uint32_t *data)
// {
//     *data =EEPROM.readInt(addr);
//     return 1;
// }
int Eeprom_Is_Init_Value(void)
{
    char tmp[IS_INIT_SIZE+1]={0};
    Read_Form_eeprom(IS_INIT_ADDR,tmp,IS_INIT_SIZE);
    //EEPROM.readString(IS_INIT_ADDR,tmp,IS_INIT_SIZE);
    //Debug.printf("tmp read:%s\n",tmp);
    if(strstr(tmp,(char *)INIT_VALUE))
        return 1;
    else 
        return 0;
}
void Set_Eeprom_Is_Init_Value(void)
{
    char tmp[IS_INIT_SIZE];
    sprintf(tmp,"%s",INIT_VALUE);
    // EEPROM.writeString(IS_INIT_ADDR,tmp);
    // EEPROM.commit();
    Write_to_eeprom(IS_INIT_ADDR,tmp,IS_INIT_SIZE);
}
void Eeprom_Set_IDpartner(int Id) {
  EEPROM.write(ID_PARTNER_ADDR , Id); 
  EEPROM.commit();
}

int Eeprom_Get_IDpartner() {
  return int(EEPROM.read(ID_PARTNER_ADDR));
}

void Eeprom_Set_Wifi_status(int Wifi_status) {
  EEPROM.write(WIFI_STATUS_ADDR , Wifi_status); 
  EEPROM.commit();
}

int Eeprom_Get_Wifi_status() {
  return int(EEPROM.read(WIFI_STATUS_ADDR));
}

void Eeprom_Set_Ssid_Pass(char *qsid, char *qpass)
{

    Write_to_eeprom(SSID_ADDR_START,qsid,strlen(qsid));

    Write_to_eeprom(PASS_ADDR_START,qpass,strlen(qpass));
	// EEPROM.commit();
}

// void Eeprom_Reset_Ssid_Pass()
// {
//     for (int i = 0; i < SSID_ADDR_SIZE; i++)
//     {
//         EEPROM.write(i+SSID_ADDR_START, 255);
//     }
// 	for (int i = 0; i < PASS_ADDR_SIZE; i++)
//     {
//         EEPROM.write(PASS_ADDR_START +i, 255);
//     }    
// 	EEPROM.commit();
// }

int Eeprom_Get_Ssid(char *ssid)
{
    // int i;
    // for (i = 0; i < SSID_ADDR_SIZE; i++)
	// {
    //     if ((ssid[i]=EEPROM.read(i+SSID_ADDR_START)) == 255)
    //         break;
        
	// }
    int len = Read_Form_eeprom(SSID_ADDR_START,ssid,SSID_ADDR_SIZE);
    ssid[len]=0;
    return len;
}

int Eeprom_Get_Password(char *pass)
{
    // int i;
    // for (i = 0; i < PASS_ADDR_SIZE; i++)
    // {
    //     if ((pass[i] = EEPROM.read(i+PASS_ADDR_START)) == 255)
    //         break;
        
    // }
    // pass[i]=0;
    // return i;
    int len = Read_Form_eeprom(PASS_ADDR_START,pass,PASS_ADDR_SIZE);
    pass[len]=0;
    return len;
}

void Eeprom_Set_Url(char *url)
{
    // for (int i = 0; i < strlen(url); i++)
    // {
    //     EEPROM.write(i + URL_ADDR_START, url[i]);
    //     EEPROM.write(i + URL_ADDR_START+1, 255);
    //     //Debug.printf("addr: %d,data: %c\n",PASS_ADDR_START+i,url[i]);
    // }
	// EEPROM.commit();
    Write_to_eeprom(URL_ADDR_START,url,strlen(url));
}

// void Eeprom_Reset_Url()
// {
//     for (int i = URL_ADDR_START; i < URL_ADDR_START + URL_ADDR_SIZE; i++)
//     {
//         EEPROM.write(i, 255);
//     }
//     EEPROM.commit();
// }

int Eeprom_Get_Url(char *url)
{
    // int i;
    // for ( i = 0; i <  URL_ADDR_SIZE; i++)
	// {
    //     if ((url[i] = EEPROM.read(i+URL_ADDR_START)) == 255)
    //         break;
	// }
    // url[i]=0;
	// return i;
    int len = Read_Form_eeprom(URL_ADDR_START,url,URL_ADDR_SIZE);
    url[len]=0;
    return len;
}


void Eeprom_Set_phoneNum(char *phone)
{
    // for (int i = 0; i < strlen(phone); i++)
    // {
    //     EEPROM.write(i + PHONE_ADDR_START, phone[i]);
    //     EEPROM.write(i + PHONE_ADDR_START+1, 255);
    //     //Debug.printf("addr: %d,data: %c\n",PASS_ADDR_START+i,url[i]);
    // }
	// EEPROM.commit();
    Write_to_eeprom(PHONE_ADDR_START,phone,strlen(phone));
}

int Eeprom_Get_phoneNum(char *phone)
{
    // int i;
    // for ( i = 0; i <  PHONE_ADDR_SIZE; i++)
	// {
    //     if ((phone[i] = EEPROM.read(i+PHONE_ADDR_START)) == 255)
    //         break;
	// }
    // phone[i]=0;
	// return i;
    int len = Read_Form_eeprom(PHONE_ADDR_START,phone,PHONE_ADDR_SIZE);
    phone[len]=0;
    
    return len;
}

void Eeprom_Set_call_default(char *call_str)
{

    Write_to_eeprom(CALL_DEFAULT_ADDR_START,call_str,strlen(call_str));
}


int Eeprom_Get_call_default(char *call_str)
{

    int len = Read_Form_eeprom(CALL_DEFAULT_ADDR_START,call_str,CALL_DEFAULT_SIZE);
    call_str[len]=0;
    return len;
}
void Eeprom_Set_result_call(uint32_t cnt,uint32_t tim)
{
    EEPROM.writeUInt(CAll_TO_DEFAULT_CNT_ADDR,cnt);
    EEPROM.writeUInt(CAll_TO_DEFAULT_TIME_ADDR,tim);
    EEPROM.commit();
}
void Eeprom_Get_result_call(uint32_t *cnt,uint32_t *tim)
{
    *cnt = EEPROM.readInt(CAll_TO_DEFAULT_CNT_ADDR);
    *tim = EEPROM.readInt(CAll_TO_DEFAULT_TIME_ADDR);
}
