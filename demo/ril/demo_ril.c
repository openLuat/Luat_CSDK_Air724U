
#include "string.h"
#include "stdio.h"
#include "at_process.h"
#include "at_tok.h"
#include "demo_ril.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"

#define AM_MS_GSM_HANDLE (1)
static HANDLE g_s_gsmMutex = NULL;
static HANDLE g_s_demoRilTask = NULL;



static bool gsmGetStatusISP(char* gmsInfoOut)
{
  int err;
  ATResponse *p_response = NULL;
  char* line = NULL;
  char *p_out = NULL;
  p_response = NULL;
  //UINT8 index = 0;
  int temp;

  static bool ispNumberType = FALSE;

  if(!ispNumberType)
  {
    err = at_send_command("AT+COPS=0,2", NULL);
    if(err < 0)
    {
      iot_debug_print("gsmGetStatusISP err line %d",__LINE__);
      goto error;
    }
    else
    {
      ispNumberType = TRUE;
    }
  }
            
  err = at_send_command_singleline("AT+COPS?", "+COPS:", &p_response);
  if (err < 0 || p_response->success == 0){
    goto error;
  }

  line = p_response->p_intermediates->line;  

  if (0 == strncmp("ABORTED", line, strlen("ABORTED")))
  {
    goto error;
  }
  else
  {

    err = at_tok_start(&line);
    if (err < 0){
        goto error;
    }
    //mode
    err = at_tok_nextint(&line,&temp);
    if (err < 0){
        goto error;
    }
    //format
    err = at_tok_nextint(&line,&temp);
    if (err < 0){
        goto error;
    }
    /*long name*/
    err = at_tok_nextstr(&line,&p_out);
    
    if (err < 0){
        goto error;
    }
    strcpy(gmsInfoOut,p_out);
  }
              
  at_response_free(p_response);
  return TRUE;

error:
  at_response_free(p_response);
  return FALSE;
}


bool gsmGetStatusCNUM(char* gmsInfoOut)
{
  int err;
  ATResponse *p_response = NULL;
  char* line = NULL;
  char *p_out = NULL;
  //UINT8 index = 0;
  bool result = FALSE;
            
  err = at_send_command_singleline("AT+CNUM", "+CNUM:", &p_response);
  if (err < 0 || p_response->success == 0){
    
    result = FALSE;
    goto end;
  }

  line = p_response->p_intermediates->line;

  if (0 == strncmp("ABORTED", line, strlen("ABORTED")))
  {
    result = FALSE;
    goto end;
  }
  else
  {  
    err = at_tok_start(&line);
    if (err < 0){
        result = FALSE;
        goto end;
    }

    //alpha
    err = at_tok_nextstr(&line,&p_out);
    if (err < 0){
        result = FALSE;
        goto end;
    }
    
    //number
    err = at_tok_nextstr(&line,&p_out);
    
    if (err < 0){
        result = FALSE;
        goto end;
    }
    strcpy(gmsInfoOut,p_out);
  }
  result = TRUE;
end:              
  at_response_free(p_response);
  return result;
}

static bool gsmGetICCID(char* iccidOut)
{
    int err;
    ATResponse *p_response = NULL;
    char* line = NULL;
    //UINT8 index = 0;
    bool result = FALSE;
    if(!iccidOut)
    {
      return result;
    }
              
    err = at_send_command_numeric("AT+CCID", &p_response);
    if (err < 0 || p_response->success == 0){
      result = FALSE;
      goto end;
    }
  
    line = p_response->p_intermediates->line;
  
    {  
      strcpy(iccidOut,line);
    }
    result = TRUE;
  end:              
    at_response_free(p_response);
    return result;


}
static bool gsmGetIMSI(char* imsiOut)
{
    int err;
    ATResponse *p_response = NULL;
    char* line = NULL;
    //UINT8 index = 0;
    bool result = FALSE;
    if(!imsiOut)
    {
      return result;
    }

    err = at_send_command_numeric("AT+CIMI", &p_response);
    if (err < 0 || p_response->success == 0){
      result = FALSE;
      goto end;
    }

    line = p_response->p_intermediates->line;

    {
      strcpy(imsiOut,line);
    }
    result = TRUE;
  end:
    at_response_free(p_response);
    return result;

}


static bool gsmLocalIP(char* outIp)
{
    int err;
    ATResponse *p_response = NULL;
    char* line = NULL;
    //UINT8 index = 0;
    bool result = FALSE;

    char *p_out = NULL;
    int temp;
              
    err = at_send_command_multiline("AT+CGDCONT?", "+CGDCONT", &p_response);
    if (err < 0 || p_response->success == 0){
      result = FALSE;
      iot_debug_print("gsmLocalIP error %d",__LINE__);
      goto end;
    }
    line = p_response->p_intermediates->line;  

    err = at_tok_start(&line);
    if (err < 0){
        goto end;
    }
    //cid
    err = at_tok_nextint(&line,&temp);
    if (err < 0){
        goto end;
    }
    //["IP"]
    err = at_tok_nextstr(&line,&p_out);
    if (err < 0){
        goto end;
    }
    //APN
    err = at_tok_nextstr(&line,&p_out);
    
    if (err < 0){
        goto end;
    }
    //IP
    err = at_tok_nextstr(&line,&p_out);
    
    if (err < 0){
        goto end;
    }
    strcpy(outIp,p_out);
              
    result = TRUE;
  end:              
    at_response_free(p_response);
    return result;

}

static bool gsmGetIMEI(char* imeiOut)
{
    int err;
    ATResponse *p_response = NULL;
    char* line = NULL;
    //UINT8 index = 0;
    bool result = FALSE;
    if(!imeiOut)
    {
      return result;
    }

    err = at_send_command_numeric("AT+GSN", &p_response);
    if (err < 0 || p_response->success == 0){
      result = FALSE;
      goto end;
    }

    line = p_response->p_intermediates->line;

    {
      strcpy(imeiOut,line);
    }
    result = TRUE;
  end:
    at_response_free(p_response);
    return result;

}



static VOID demoRilNetworkCb(E_OPENAT_NETWORK_STATE state)
{
        
    iot_debug_print("[RIL] %s status %d", __FUNCTION__, state);

    switch (state)
    {
    case OPENAT_NETWORK_READY:
        {
            T_OPENAT_NETWORK_CONNECT param = 
            {
                "cmnet",
                "",
                ""
            };
            iot_network_connect(&param);   
        } 
        break;
    case OPENAT_NETWORK_GOING_DOWN:
        iot_debug_print("[RIL] nw pdp deactived");
        //尝试重新连接
        T_OPENAT_NETWORK_CONNECT param = 
        {
            "cmnet",
            "",
            ""
        };
        iot_network_connect(&param);
        break;
    case OPENAT_NETWORK_DISCONNECT:
        iot_debug_print("[RIL] nw down");
        break;
    case OPENAT_NETWORK_LINKED:
        {
            iot_os_send_message(g_s_demoRilTask, 0);
        }
        break;
    default:
        break;
    }
}

void demoRilTask(void)
{
    PVOID msg;

    while (1)
    {
        iot_os_wait_message(g_s_demoRilTask, &msg);
        char strResult[64] = {0};
        gsmLocalIP(strResult);
        iot_debug_print("[ril] demoRilNetworkCb IP %s", strResult);
        gsmGetIMEI(strResult);
        iot_debug_print("[ril] demoRilNetworkCb IMEI %s", strResult);
        gsmGetICCID(strResult);
        iot_debug_print("[ril] demoRilNetworkCb ICCID %s", strResult);
        gsmGetStatusISP(strResult);
        iot_debug_print("[ril] demoRilNetworkCb ISP %s", strResult);
    }
    
    

}

int appimg_enter(void *param)
{    
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
    iot_network_set_cb(demoRilNetworkCb);

    g_s_demoRilTask = iot_os_create_task(demoRilTask,
            NULL,
            1024,
            20,
            0,
            "DEMO_RIL"

            );
    
    return 0;
}

void appimg_exit(void)
{

}


