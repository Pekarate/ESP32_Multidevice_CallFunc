#ifndef __AT_COMMAND_H
#define __AT_COMMAND_H

#include "stdint.h"

void AT_Free_rx_buffer(void);
void AT_Port_Init(int Baud);
void AT_Send(char *data);
int At_Command(char *cmd ,char *RSP1,uint32_t timeout);
int At_Command_Without_Endline(char *cmd ,char *RSP1,uint32_t timeout);
int At_Command1(char *cmd ,char *RSP1,uint32_t timeout);
int At_Command_nodebug(char *cmd ,char *RSP1,uint32_t timeout);
void User_command(uint32_t  timout) ;
int AT_read_until(uint8_t *Des,char *end,uint16_t len,uint32_t timeout);
void AT_Write(uint8_t *data,uint16_t len);

int AT_Getstring_index(char *des,char *scr,char *key,int index);
int AT_Getint_index(int *res,char *src,char *key,int index);

int AT_GetInfo(char *info);
void AT_Check_unexpected_Response(char *Bufer,uint16_t *Buf_size_control);
#define TIMEOUT -1
#define ERROR -2
#define AT_OK "OK"

#endif
