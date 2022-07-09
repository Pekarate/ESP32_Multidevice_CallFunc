#ifndef FTCEEPROM_H_
#define FTCEEPROM_H_

#include <EEPROM.h>

#define EEPROM_SIZE 256

#define WIFI_STATUS_ADDR 202

#define SSID_ADDR_START 0
#define SSID_ADDR_SIZE 32
    
#define PASS_ADDR_START 32
#define PASS_ADDR_SIZE 64

#define URL_ADDR_START 96
#define URL_ADDR_SIZE 32

#define PHONE_ADDR_START 128
#define PHONE_ADDR_SIZE 15

#define ID_PARTNER_ADDR 200
#define ID_PARTNER_SIZE 2

#define CALL_DEFAULT_ADDR_START 205
#define CALL_DEFAULT_SIZE 40

#define IS_INIT_ADDR 150

#define CAll_TO_DEFAULT_CNT_ADDR 170

#define CAll_TO_DEFAULT_TIME_ADDR 175
    
void Eeprom_Init();

int Eeprom_Is_Init_Value(void);
void Set_Eeprom_Is_Init_Value(void);
void Eeprom_Set_IDpartner(int Id);
int Eeprom_Get_IDpartner();

void Eeprom_Set_Wifi_status(int Wifi_status);
int Eeprom_Get_Wifi_status();

// void saveSim800CallMode(int index);
// void saveSim800AnswerMode(int index) ;
// int getSim800Mode( int index );

void Eeprom_Set_Ssid_Pass(char *qsid, char *qpass);
void Eeprom_Reset_Ssid_Pass();

void Eeprom_Set_phoneNum(char *phone);
int Eeprom_Get_phoneNum(char *phone);

int Eeprom_Get_Ssid(char *ssid);
int Eeprom_Get_Password(char *pass);

void Eeprom_Set_Url(char *url);
void Eeprom_Reset_Url();
int Eeprom_Get_Url(char *url);

void Eeprom_Set_rst_cnt(int cnt);
int  Eeprom_Get_rst_cnt();
void Eeprom_reset_cnt();

void Eeprom_Set_call_default(char *call_str);
int Eeprom_Get_call_default(char *call_str);

void Eeprom_Set_result_call(uint32_t cnt,uint32_t tim);
void Eeprom_Get_result_call(uint32_t *cnt,uint32_t *tim);
#endif
