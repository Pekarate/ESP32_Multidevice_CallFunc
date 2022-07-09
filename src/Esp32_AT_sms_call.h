#ifndef __SIM800_SMS_CALL_H
#define __SIM800_SMS_CALL_H

int AT_Call_SetAutoAnswer(void);
int AT_Call_Hangup(void);
int AT_Call_To(char* input_number);
int AT_call_Waitresult(int Calltime);


int AT_Sms_Getlist();
int AT_Sms_Send(char* input_number,char* msg);
void AT_Sms_Delele(int index,int flag);
void AT_Sms_DeleleAll(void);

int AT_SIM7600_call_Waitresult(int Calltime);
#endif