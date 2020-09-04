
#ifndef __DEMO_RIL_H__
#define __DEMO_RIL_H__
#include "cs_types.h"

#define MS_HAL_NUM_DEFAULT_LENGTH	(32)
#define MS_HAL_GSM_DEFAULT_LENGTH	(64)
#define MS_HAL_GSM_APN_LENGTH		(64)
#define MS_HAL_GSM_USERNAME_LENGTH	(32)
#define MS_HAL_GSM_PASSWORD_LENGTH	(64)

typedef struct ST_GsmStatus{
	bool connected;							/*< is connected to data link 	*/
	bool roam;								/*< is roam 					*/
	unsigned char signal;							/*< signal strength, [0, 255) 	*/
	unsigned char gen;							/*< Generation, [2,5], 2-2G... 	*/
	char isp[MS_HAL_GSM_DEFAULT_LENGTH];	/*< isp code 					*/
	char imsi[MS_HAL_GSM_DEFAULT_LENGTH];	/*< imsi code 					*/
}T_GsmStatus;

typedef struct ST_GsmConnect{
	char dialog_num[MS_HAL_NUM_DEFAULT_LENGTH];
	char apn[MS_HAL_GSM_APN_LENGTH];
	char username[MS_HAL_GSM_USERNAME_LENGTH];
	char password[MS_HAL_GSM_PASSWORD_LENGTH];
}T_GsmConnect;



typedef struct ST_GsmInfo{
	char msisdn[MS_HAL_NUM_DEFAULT_LENGTH];		/*< msisdn 	*/
	char iccid[MS_HAL_NUM_DEFAULT_LENGTH];		/*< iccid 	*/
	char isp_code[MS_HAL_NUM_DEFAULT_LENGTH];	/*< isp_code 	*/
}T_GsmInfo;

typedef struct 
{
	char ipString[16];	/*< ip string	*/
}T_GsmInfoIp;

typedef struct
{
	char imei[16];	/*< imei string	*/
}T_GsmImei;

#ifdef  __cplusplus
extern  "C"
{
#endif

#ifdef  __cplusplus
}
#endif


#endif /* __MS_HAL_GSM_H__ */
