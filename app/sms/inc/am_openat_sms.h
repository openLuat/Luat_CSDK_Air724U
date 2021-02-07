

#ifndef AM_OPENAT_SMS_H
#define AM_OPENAT_SMS_H

typedef void (*RecSmsHandlerCb)(int status, char* data, char* num, char * datetime);

bool sms_init(void);
BOOL sms_send(char* num, char* data);
void sms_setsmscb(RecSmsHandlerCb cb);

#endif 

