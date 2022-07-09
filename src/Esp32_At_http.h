#ifndef __SIM800A_HTTP_H
#define __SIM800A_HTTP_H

int AT_Http_Request(char * request_url,char *rsp);
int AT_HTTP_Post(char * request_url);
int AT_Http_Read_Contentlength();
int AT_Http_Read_Statuscode();
int AT_Http_Read_Data(char *des,int len);
int AT_UC15_HTTP_Get(char* url,char *res);
int AT_SIM7600_HTTP_Get(char * request_url,char *rsp);
#endif