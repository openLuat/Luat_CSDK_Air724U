/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_watchdog.h
 * Author:  zhutianhua
 * Version: V0.1
 * Date:    2014/8/6
 *
 * Description:
 *          platform gpscore ½Ó¿Ú
 **************************************************************************/

#ifndef _PLATFORM_GPS_H_
#define _PLATFORM_GPS_H_

#define GPS_MODE_RAW_DATA (0)
#define GPS_MODE_LOCATION (1)
#define GPS_MODE_LOCATION_WITH_QOP (2)


typedef enum
{
    PLATFORM_GPS_PARSER_RAW_DATA = 0,    /*Raw data of NMEA*/
    PLATFORM_GPS_PARSER_NMEA_GGA,        /*Data structure of GGA info*/
    PLATFORM_GPS_PARSER_NMEA_GLL,        /*Data structure of GLL info*/
    PLATFORM_GPS_PARSER_NMEA_GSA,        /*Data structure of GSA info*/
    PLATFORM_GPS_PARSER_NMEA_GSV,        /*Data structure of GSV info*/
    PLATFORM_GPS_PARSER_NMEA_RMC,        /*Data structure of RMC info*/
    PLATFORM_GPS_PARSER_NMEA_VTG,        /*Data structure of VTG info*/
    PLATFORM_GPS_PARSER_NMEA_GAGSA,       /*Data structure of GAGSA info*/
    PLATFORM_GPS_PARSER_NMEA_GAGSV,       /*Data structure of GAGSV info*/
    PLATFORM_GPS_PARSER_NMEA_GLGSA,       /*Data structure of GLGSA info*/
    PLATFORM_GPS_PARSER_NMEA_GLGSV,       /*Data structure of GLGSV info*/
    PLATFORM_GPS_PARSER_NMEA_SENTENCE,
    PLATFORM_GPS_UART_EVENT_VPORT_LOST,  /*Virtual port is lost, maybe bluetooth connection is break(not support current)*/
    PLATFORM_GPS_SHOW_AGPS_ICON,
    PLATFORM_GPS_HIDE_AGPS_ICON,
    PLATFORM_GPS_PARSER_NMEA_ACC,        /*Data structure of ACCURACY info*/
    PLATFORM_GPS_PARSER_NMEA_END,
    PLATFORM_GPS_OPEN_IND,
    PLATFORM_GPS_UART_RAW_DATA,
    PLATFORM_GPS_PARSER_MA_STATUS = 255
}E_PLATFROM_GPS_DATA_TYPE; //==E_AMOPENAT_DATA_TYPE



typedef enum
{
    PLATFORM_GPS_UART_GPS_WARM_START = 0,        /*Let GPS do warm start*/
    PLATFORM_GPS_UART_GPS_HOT_START,             /*Let GPS do hot start*/
    PLATFORM_GPS_UART_GPS_COLD_START,            /*Let GPS do cold start*/
    PLATFORM_GPS_UART_GPS_VERSION,
    PLATFORM_GPS_UART_GPS_ENABLE_DEBUG_INFO,    
    PLATFORM_GPS_UART_GPS_SWITCH_MODE_MA,
    PLATFORM_GPS_UART_GPS_SWITCH_MODE_MB,
    PLATFORM_GPS_UART_GPS_SWITCH_MODE_NORMAL,
    PLATFORM_GPS_UART_GPS_QUERY_POS,
    PLATFORM_GPS_UART_GPS_QUERY_MEAS,
    PLATFORM_GPS_UART_GPS_CLEAR_NVRAM,           /*Clear GPS NVRAM*/
    PLATFORM_GPS_UART_GPS_AGPS_START,            /*Clear GPS data*/
    PLATFORM_GPS_UART_GPS_SLEEP,                 /*Let GPS chip goto sleep mode*/
    PLATFORM_GPS_UART_GPS_STOP,                  /*Let GPS chip stop*/
    PLATFORM_GPS_UART_GPS_WAKE_UP,               /*Let GPS chip wake up from sleep mode*/
    PLATFORM_GPS_UART_GPS_DUMMY = -1
}T_PLATFORM_GPS_CMD;


void platform_gps_init(void);
int platform_gps_open(UINT8 mode);

int platform_gps_close(void);

int platform_gps_read(UINT8* data);

int platform_gps_write(VOID* buf, UINT32 len);

int platform_gps_cmd(T_PLATFORM_GPS_CMD cmd);



#endif//_PLATFORM_GPS_H_

