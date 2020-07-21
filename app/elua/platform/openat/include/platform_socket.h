#ifndef __PLATFORM_SOCKET_H__
#define __PLATFORM_SOCKET_H__

#include "am_openat_socket.h"

#define CUSTOM_DTCNT_PROF_MAX_USER_LEN (32-1)
#define CUSTOM_DTCNT_PROF_MAX_PW_LEN (32 - 1)

#define MAX_APN_LEN             100
/* Socket Type */




/*
 * <GROUP  Macro_Consts>
 * 
 * max socket address length 
 */
#define MAX_SOCK_ADDR_LEN           (100)
#define LOCAL_PARA_HDR



typedef struct
{
   kal_uint32 account_id;
   kal_bool result;
   kal_int8 sock_id;
   kal_uint32    user_data;
} mthl_create_conn_cnf_struct;

typedef struct
{
   LOCAL_PARA_HDR 
   kal_uint32 account_id;
   kal_int8 sock_id;
   kal_uint32    user_data;
} mthl_create_sock_ind_struct;

typedef enum
{
    SUCCESS,
    PARAM_INVALID,
	LIMITED_RES,	
}mthl_create_pdp_param_result;

typedef struct
{
    LOCAL_PARA_HDR
    //kal_bool       result;
    mthl_create_pdp_param_result    result;
    kal_uint32     account_id;//ori_acct_id
    kal_uint32     user_data;
}mthl_create_pdp_param_cnf_struct;


typedef struct
{
    LOCAL_PARA_HDR
    kal_bool       result;
    kal_uint32     error_cause;
    kal_uint32     account_id;
    kal_uint32     user_data;
} mthl_activate_pdp_cnf_struct;

typedef struct
{
  LOCAL_PARA_HDR
  kal_uint8 sock_id;
  kal_bool result;//是否成功；
  kal_int32 ret_val;//实际送到TCP发送Buffer中的data size
  kal_uint32    user_data;
} mthl_send_data_cnf_struct, mthl_send_data_ind_struct;

typedef struct
{
  LOCAL_PARA_HDR 
  kal_uint32     account_id;
  kal_bool       result;
  kal_uint32     error_cause;
  kal_uint32     user_data;
} mthl_deactivate_pdp_cnf_struct;

typedef struct
{ 
    LOCAL_PARA_HDR
    kal_uint32     account_id;
    int error;
    kal_uint32 error_cause;
} mthl_deactivate_pdp_ind_struct;

typedef struct
{ 
  LOCAL_PARA_HDR
  kal_bool result;
  kal_uint8 sock_id;
  kal_uint32    user_data;
} mthl_close_sock_ind_struct;

typedef struct
{ 
  LOCAL_PARA_HDR
  kal_bool result;//关闭是否成功；
  kal_uint8 sock_id;//关闭的Sockets ID；
  kal_uint32    user_data;
} mthl_close_sock_cnf_struct;

typedef struct
{
  char* hostName;
  char* serverCacert;  
  char* clientCacert;
  char* clientKey;
}mthl_socket_cert;


void platform_lua_socket_init(void);

kal_bool platform_activate_pdp(kal_char* apn, 
                                      kal_char* user_name, 
                                      kal_char* password);


kal_bool platform_deactivate_pdp(void);



kal_bool platform_socket_send(kal_uint8 socket_index,
                                       kal_uint8*	data,                                         
                                       kal_uint16	length);


kal_int32 platform_socket_recv(kal_uint8 socket_index,
                                       kal_uint8*	data,                                         
                                       kal_uint16	length);

kal_bool platform_socket_close(kal_uint8 socket_index);


int platform_socket_open(
                                         openSocketType sock_type,
                                         kal_uint16    port,
                                         kal_char*    addr,
                                         mthl_socket_cert* cert);

kal_int32 platform_on_create_conn_cnf(lua_State *L,
                                             PlatformSocketConnectCnf* create_conn_cnf);


kal_int32 platform_on_create_sock_ind(lua_State *L,
                                             mthl_create_sock_ind_struct* create_sock_ind);


kal_int32 platform_on_create_pdp_param_cnf(lua_State *L,
                                                     mthl_create_pdp_param_cnf_struct* create_pdp_param_cnf);



kal_int32 platform_on_active_pdp_cnf(lua_State *L,
                                           PlatformPdpActiveCnf* active_pdp_cnf);



kal_int32 platform_on_send_data_cnf(lua_State *L,
                                          PlatformSocketSendCnf* send_data_cnf);



kal_int32 platform_on_send_data_ind(lua_State *L,
                                          mthl_send_data_ind_struct* send_data_ind);



kal_int32 platform_on_recv_data_ind(lua_State *L,
                                          PlatformSocketRecvInd* recv_data_ind);



kal_int32 platform_on_deactivate_pdp_cnf(lua_State *L,
                                                 mthl_deactivate_pdp_cnf_struct* deactivate_pdp_cnf);



kal_int32 platform_on_deactivate_pdp_ind(lua_State *L,
                                                 mthl_deactivate_pdp_ind_struct* deactivate_pdp_ind);



kal_int32 platform_on_sock_close_ind(lua_State *L,
                                            PlatformSocketCloseInd* sock_close_ind);



kal_int32 platform_on_sock_close_cnf(lua_State *L,
                                           PlatformSocketCloseCnf* sock_close_cnf);



void platform_socket_on_recv_done(kal_uint8 socket_index, kal_uint32 recv_len);


#endif

