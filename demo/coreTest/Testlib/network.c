#include "string.h"
#include "iot_os.h"
#include "iot_network.h"
bool networkstatus = FALSE;

static void app_networkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
	iot_debug_print("[coreTest-network]: E_OPENAT_NETWORK_STATE state:%d", state);
	if (state == OPENAT_NETWORK_READY)
	{
		T_OPENAT_NETWORK_CONNECT networkparam;
		memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
		memcpy(networkparam.apn, "CMNET", strlen("CMNET"));
		iot_network_connect(&networkparam);
		return;
	}
	else if (state == OPENAT_NETWORK_LINKED)
		networkstatus = TRUE;
	else if (state == OPENAT_NETWORK_DISCONNECT)
		networkstatus = FALSE;
}

void networkTest(void)
{
	iot_network_set_cb(app_networkIndCallBack);
}
