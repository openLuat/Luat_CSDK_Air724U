#include <stdio.h>
#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_pmd.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "httpclient.h"
#include "am_openat_httpclient.h"

#define SOCKET_MSG_NETWORK_READY (0)
#define SOCKET_MSG_NETWORK_LINKED (1)
#define HTTP_URL "http://download.openluat.com:80/9501-xingli/brdcGPD.dat_rda"
#define HEAD_ACCEPT_KEY "Accept"
#define HEAD_ACCEPT_VALUE "*/*"
#define HEAD_ACCEPT_L_KEY "Accept-Language"
#define HEAD_ACCEPT_L_VALUE "cn"
#define HEAD_USER_KEY "User-Agent"
#define HEAD_USER_VALUE "*Mozilla/4.0"
#define HEAD_CONNECTION_KEY "Connection"
#define HEAD_CONNECTION_VALUE "Keep-Alive"

static void http_debug(const char *fun, const char *data, UINT32 len, char *fmt, ...)
{
	va_list ap;
	char fmtString[128] = {0};
	UINT16 fmtStrlen;
	strcat(fmtString, "[coreTest-http] :--");
	strcat(fmtString, fun);
	strcat(fmtString, "--");

	fmtStrlen = strlen(fmtString);
	va_start(ap, fmt);
	fmtStrlen += vsnprintf(fmtString + fmtStrlen, sizeof(fmtString) - fmtStrlen, fmt, ap);
	va_end(ap);

	if (fmtStrlen != 0)
	{
		iot_debug_print("%s", fmtString);
	}
}

void http_test(void)
{
	HTTP_SESSION_HANDLE pHTTP;
	CHAR readBuff[1460];
	UINT32 readSize = 0;
	UINT32 readTotalLen = 0;
	CHAR token[32];
	UINT32 tokenSize = 32;
	UINT32 nRetCode;

	pHTTP = HTTPClientOpenRequest(0);

	HTTPClientSetDebugHook(pHTTP, http_debug);

	if (HTTPClientSetVerb(pHTTP, VerbGet) != HTTP_CLIENT_SUCCESS)
	{
		iot_debug_print("[coreTest-False-http] : HTTPClientSetVerb error");
		return;
	}

	if ((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_ACCEPT_KEY, HEAD_ACCEPT_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
	{
		return;
	}
	if ((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_ACCEPT_L_KEY, HEAD_ACCEPT_L_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
	{
		return;
	}
	if ((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_USER_KEY, HEAD_USER_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
	{
		return;
	}
	if ((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_CONNECTION_KEY, HEAD_CONNECTION_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
	{
		return;
	}
	iot_debug_print("[coreTest-http] : HTTPClientSendRequest enter");
	if (HTTPClientSendRequest(pHTTP, HTTP_URL, NULL, 0, TRUE, 0, 0) != HTTP_CLIENT_SUCCESS)
	{
		iot_debug_print("[coreTest-False-http] : HTTPClientSendRequest error");
		return;
	}
	iot_debug_print("[coreTest-http] : HTTPClientRecvResponse enter");

	if (HTTPClientRecvResponse(pHTTP, 20000) != HTTP_CLIENT_SUCCESS)
	{
		iot_debug_print("[coreTest-False-http] : HTTPClientRecvResponse error");
		return;
	}

	if ((nRetCode = HTTPClientFindFirstHeader(pHTTP, "content-length", token, &tokenSize)) != HTTP_CLIENT_SUCCESS)
	{
		iot_debug_print("[coreTest-False-http] : HTTPClientFindFirstHeader error");
		return;
	}
	else
	{
		iot_debug_print("[coreTest-http] : HTTPClientFindFirstHeader %d,%s", tokenSize, token);
	}
	HTTPClientFindCloseHeader(pHTTP);

	while (nRetCode == HTTP_CLIENT_SUCCESS || nRetCode != HTTP_CLIENT_EOS)
	{
		// Set the size of our buffer

		// Get the data
		nRetCode = HTTPClientReadData(pHTTP, readBuff, sizeof(readBuff), 300, &readSize);

		readTotalLen += readSize;
		if (nRetCode != HTTP_CLIENT_SUCCESS || nRetCode == HTTP_CLIENT_EOS)
		{
			iot_debug_print("[coreTest-http] : HTTPClientReadData end nRetCode %d", nRetCode);
			break;
		}

		iot_debug_print("[coreTest-http] : HTTPClientReadData readTotalLen %d, %d, nRetCode %d", readTotalLen, readSize, nRetCode);
	}

	if (HTTPClientCloseRequest(&pHTTP) != HTTP_CLIENT_SUCCESS)
	{
		iot_debug_print("[coreTest-False-http] : HTTPIntrnConnectionClose error");
		return;
	}
}

void httpTest(void)
{
	extern bool networkstatus;
	if (networkstatus == FALSE)
		return FALSE;
	http_test();
}
