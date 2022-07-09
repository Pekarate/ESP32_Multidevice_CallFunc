#ifndef __SIM800_COMMON_H
#define __SIM800_COMMON_H

int AT_Device_Init(int baud);
int AT_Get_Module_IMEI(char *Imei);
int AT_Get_Sim_IMEI(char *Imei);
#endif
