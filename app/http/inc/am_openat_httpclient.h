/***************************************************************************** 
* 
* File Name : wm_http_client.h
* 
* Description: Http client header file.
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : wanghf 
* 
* Date : 2014-6-6 
*****************************************************************************/ 

#ifndef AM_OPENAT_HTTPCLIENT_H
#define AM_OPENAT_HTTPCLIENT_H
#include "iot_socket.h"
#include <stdlib.h>
#include <ctype.h>
#include "string.h"

#ifndef ULONG
#define ULONG						unsigned long
#endif

#undef ULONG_MAX
#define ULONG_MAX                   0xffffffff


#define CFG_ON 1
#define CFG_OFF 0
#define TLS_CONFIG_HTTP_CLIENT CFG_ON
#define TLS_CONFIG_HTTP_CLIENT_PROXY		CFG_OFF
#define TLS_CONFIG_HTTP_CLIENT_AUTH_BASIC	CFG_OFF
#define TLS_CONFIG_HTTP_CLIENT_AUTH_DIGEST	CFG_OFF
#define TLS_CONFIG_HTTP_CLIENT_AUTH			(TLS_CONFIG_HTTP_CLIENT_AUTH_BASIC || TLS_CONFIG_HTTP_CLIENT_AUTH_DIGEST)
#define TLS_CONFIG_HTTP_CLIENT_SECURE		CFG_OFF
#define TLS_CONFIG_HTTP_CLIENT_TASK			CFG_OFF 
#define _HTTP_DEBUGGING_
#define MEMCPY memcpy



#define VOID                         void
#define HZ 16384
#define TLS_DBGPRT_INFO  iot_debug_print
#define tls_mem_free iot_os_free
#define tls_mem_alloc iot_os_malloc
#define tls_os_get_time iot_os_get_system_tick
#define closesocket close
#define shutdown 
// HTTP Status, API Return codes
#define HTTP_CLIENT_SUCCESS                 0 // HTTP Success status

#define HTTP_CLIENT_UNKNOWN_ERROR           1 // Unknown error
#define HTTP_CLIENT_ERROR_INVALID_HANDLE    2  // an Invalid handle or possible bad pointer was passed to a function
#define HTTP_CLIENT_ERROR_NO_MEMORY         3  // Buffer too small or a failure while in memory allocation
#define HTTP_CLIENT_ERROR_SOCKET_INVALID    4  // an attempt to use an invalid socket handle was made
#define HTTP_CLIENT_ERROR_SOCKET_CANT_SET   5  // Can't send socket parameters
#define HTTP_CLIENT_ERROR_SOCKET_RESOLVE    6  // Error while resolving host name
#define HTTP_CLIENT_ERROR_SOCKET_CONNECT    7  // Error while connecting to the remote server
#define HTTP_CLIENT_ERROR_SOCKET_TIME_OUT   8  // socket time out error
#define HTTP_CLIENT_ERROR_SOCKET_RECV       9  // Error while receiving data
#define HTTP_CLIENT_ERROR_SOCKET_SEND       10 // Error while sending data
#define HTTP_CLIENT_ERROR_HEADER_RECV       11 // Error while receiving the remote HTTP headers
#define HTTP_CLIENT_ERROR_HEADER_NOT_FOUND  12 // Could not find element within header
#define HTTP_CLIENT_ERROR_HEADER_BIG_CLUE   13 // The headers search clue was too large for the internal API buffer
#define HTTP_CLIENT_ERROR_HEADER_NO_LENGTH  14 // No content length was specified for the outgoing data. the caller should specify chunking mode in the session creation
#define HTTP_CLIENT_ERROR_CHUNK_TOO_BIG     15 // The HTTP chunk token that was received from the server was too big and possibly wrong
#define HTTP_CLIENT_ERROR_AUTH_HOST         16 // Could not authenticate with the remote host
#define HTTP_CLIENT_ERROR_AUTH_PROXY        17 // Could not authenticate with the remote proxy
#define HTTP_CLIENT_ERROR_BAD_VERB          18 // Bad or not supported HTTP verb was passed to a function
#define HTTP_CLIENT_ERROR_LONG_INPUT        19 // a function received a parameter that was too large
#define HTTP_CLIENT_ERROR_BAD_STATE         20 // The session state prevents the current function from proceeding
#define HTTP_CLIENT_ERROR_CHUNK             21 // Could not parse the chunk length while in chunked transfer
#define HTTP_CLIENT_ERROR_BAD_URL           22 // Could not parse curtail elements from the URL (such as the host name, HTTP prefix act')
#define HTTP_CLIENT_ERROR_BAD_HEADER        23 // Could not detect key elements in the received headers
#define HTTP_CLIENT_ERROR_BUFFER_RSIZE      24 // Error while attempting to resize a buffer
#define HTTP_CLIENT_ERROR_BAD_AUTH          25 // Authentication schema is not supported
#define HTTP_CLIENT_ERROR_AUTH_MISMATCH     26 // The selected authentication schema does not match the server response
#define HTTP_CLIENT_ERROR_NO_DIGEST_TOKEN   27 // an element was missing while parsing the digest authentication challenge
#define HTTP_CLIENT_ERROR_NO_DIGEST_ALG     28 // Digest algorithem could be MD5 or MD5-sess other types are not supported
#define HTTP_CLIENT_ERROR_SOCKET_BIND		  29 // Binding error
#define HTTP_CLIENT_ERROR_TLS_NEGO			  30 // Tls negotiation error
#define HTTP_CLIENT_ERROR_NOT_IMPLEMENTED   64 // Feature is not (yet) implemented
#define HTTP_CLIENT_EOS                     1000        // HTTP end of stream message

// HTTP Type Definitions 
typedef UINT32          HTTP_SESSION_HANDLE;
typedef UINT32          HTTP_CLIENT_SESSION_FLAGS;


///////////////////////////////////////////////////////////////////////////////
//
// Section      : HTTP API structures
//
///////////////////////////////////////////////////////////////////////////////

// HTTP Supported authentication methods 
typedef enum _HTTP_AUTH_SCHEMA
{
    AuthSchemaNone      = 0,
    AuthSchemaBasic,
    AuthSchemaDigest,
    AuthSchemaKerberos,
    AuthNotSupported

} HTTP_AUTH_SCHEMA;

// HTTP supported verbs
typedef enum _HTTP_VERB
{
    VerbGet             = 0,
    VerbHead,
    VerbPost,
    VerbPut,
    VerbNotSupported
    // Note: others verb such as connect and put are currently not supported

} HTTP_VERB;
// Data structure that the caller can request at any time that will include some information regarding the session
typedef struct _HTTP_CLIENT
{
    UINT32        HTTPStatusCode;                 // HTTP Status code (200 OK)
    UINT32		    RequestBodyLengthSent;          // Total bytes sent (body only)
    UINT32		    ResponseBodyLengthReceived;     // Total bytes received (body only)
    UINT32		    TotalResponseBodyLength;        // as extracted from the “content-length" header
    UINT32        HttpState;
} HTTP_CLIENT;


typedef struct _HTTPParameters
{
    CHAR*                  Uri;        
    CHAR*                  ProxyHost;  
    UINT32                  UseProxy ;  
    UINT32                  ProxyPort;
    UINT32                  Verbose;
    CHAR*                  UserName;
    CHAR*                  Password;
    HTTP_AUTH_SCHEMA      AuthType;

} HTTPParameters;

///////////////////////////////////////////////////////////////////////////////
//
// Section      : HTTP API public interface
//
///////////////////////////////////////////////////////////////////////////////


/*************************************************************************** 
* Function: HTTPClientOpenRequest 
* Description: Allocate memory for a new HTTP Session. 
* 
* Input: Flags: HTTP Session internal API flags, 0 should be passed here. 
* 
* Output: None 
* 
* Return: HTTP Session handle
* 
* Date : 2014-6-6 
****************************************************************************/ 
HTTP_SESSION_HANDLE     HTTPClientOpenRequest         (HTTP_CLIENT_SESSION_FLAGS Flags);
/*************************************************************************** 
* Function: HTTPClientCloseRequest 
* Description: Closes any active connection and free any used memory. 
* 
* Input: pSession: HTTP Session handle. 
* 
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientCloseRequest        (HTTP_SESSION_HANDLE *pSession);
/*************************************************************************** 
* Function: HTTPClientSetAuth 
* Description: Sets the HTTP authentication schema. 
* 
* Input: pSession:       HTTP Session handle. 
*           AuthSchema: HTTP Supported authentication methods.
*           pReserved:    Reserved parameter.
* 
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientSetAuth             (HTTP_SESSION_HANDLE pSession, HTTP_AUTH_SCHEMA AuthSchema, void *pReserved);
/*************************************************************************** 
* Function: HTTPClientSetCredentials 
* Description: Sets credentials for the target host. 
* 
* Input: pSession: HTTP Session handle. 
*           pUserName:   User name.
*           pPassword:    Password.
* 
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientSetCredentials      (HTTP_SESSION_HANDLE pSession, CHAR *pUserName, CHAR *pPassword);
/*************************************************************************** 
* Function: HTTPClientSetProxy 
* Description: Sets all the proxy related parameters. 
* 
* Input: pSession: HTTP Session handle. 
*           pProxyName: The host name.
*           nPort:            The proxy port number.
*           pUserName:   User name for proxy authentication (can be null).
*           pPassword:    User password for proxy authentication (can be null).
* 
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientSetProxy            (HTTP_SESSION_HANDLE pSession, CHAR *pProxyName, UINT16 nPort, CHAR *pUserName, CHAR *pPassword);
/*************************************************************************** 
* Function: HTTPClientSetVerb 
* Description: Sets the HTTP verb for the outgoing request. 
* 
* Input: pSession: HTTP Session handle. 
*           HttpVerb: HTTP supported verbs
* 
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientSetVerb             (HTTP_SESSION_HANDLE pSession, HTTP_VERB HttpVerb);
/*************************************************************************** 
* Function: HTTPClientAddRequestHeaders 
* Description: Add headers to the outgoing request. 
* 
* Input: pSession: HTTP Session handle. 
*           pHeaderName:    The Headers name
*           pHeaderData:      The headers data
*           nInsert:              Reserved could be any
* 
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientAddRequestHeaders   (HTTP_SESSION_HANDLE pSession, CHAR *pHeaderName, CHAR *pHeaderData, BOOL nInsert);
/*************************************************************************** 
* Function: HTTPClientSendRequest 
* Description: This function builds the request headers, performs a DNS resolution , 
*                 opens the connection (if it was not opened yet by a previous request or if it has closed) 
*                 and sends the request headers. 
* 
* Input: pSession: HTTP Session handle. 
*           pUrl:               The requested URL
*           pData:             Data to post to the server
*           nDataLength:   Length of posted data
*           TotalLength:     Valid only when http method is post.
*                     TRUE:   Post data to http server.
*                     FALSE: In a post request without knowing the total length in advance so return error or use chunking.
*           nTimeout:        Operation timeout
*           nClientPort:      Client side port 0 for none 
*
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientSendRequest         (HTTP_SESSION_HANDLE pSession, CHAR *pUrl, VOID *pData, UINT32 nDataLength, BOOL TotalLength, UINT32 nTimeout,UINT32 nClientPort);
/*************************************************************************** 
* Function: HTTPClientWriteData 
* Description: Write data to the remote server. 
* 
* Input: pSession: HTTP Session handle. 
*           pBuffer:             Data to write to the server.
*           nBufferLength:   Length of wtitten data.
*           nTimeout:          Timeout for the operation.
* 
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientWriteData           (HTTP_SESSION_HANDLE pSession, VOID *pBuffer, UINT32 nBufferLength, UINT32 nTimeout);
/*************************************************************************** 
* Function: HTTPClientRecvResponse 
* Description: Receives the response header on the connection and parses it.
*                Performs any required authentication. 
* 
* Input: pSession: HTTP Session handle. 
*           nTimeout:          Timeout for the operation.
* 
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientRecvResponse        (HTTP_SESSION_HANDLE pSession, UINT32 nTimeout);
/*************************************************************************** 
* Function: HTTPClientReadData 
* Description: Read data from the server. Parse out the chunks data.
* 
* Input: pSession: HTTP Session handle. 
*           nBytesToRead:    The size of the buffer (numbers of bytes to read)
*           nTimeout:           Operation timeout in seconds
* 
* Output: pBuffer:              A pointer to a buffer that will be filled with the servers response
*             nBytesRecived:   Count of the bytes that were received in this operation 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientReadData            (HTTP_SESSION_HANDLE pSession, VOID *pBuffer, UINT32 nBytesToRead, UINT32 nTimeout, UINT32 *nBytesRecived);
/*************************************************************************** 
* Function: HTTPClientGetInfo 
* Description: Fill the users structure with the session information. 
* 
* Input: pSession: HTTP Session handle. 
* 
* Output: HTTPClient:   The session information. 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientGetInfo             (HTTP_SESSION_HANDLE pSession, HTTP_CLIENT *HTTPClient);
/*************************************************************************** 
* Function: HTTPClientFindFirstHeader 
* Description: Initiate the headr searching functions and find the first header. 
* 
* Input: pSession: HTTP Session handle. 
*           pSearchClue:   Search clue.
* 
* Output: pHeaderBuffer: A pointer to a buffer that will be filled with the header name and value.
*             nLength:          Count of the bytes that were received in this operation.
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientFindFirstHeader     (HTTP_SESSION_HANDLE pSession, CHAR *pSearchClue,CHAR *pHeaderBuffer, UINT32 *nLength);
/*************************************************************************** 
* Function: HTTPClientGetNextHeader 
* Description: Find the next header.
* 
* Input: pSession: HTTP Session handle. 
* 
* Output: pHeaderBuffer: A pointer to a buffer that will be filled with the header name and value.
*             nLength:          Count of the bytes that were received in this operation. 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientGetNextHeader       (HTTP_SESSION_HANDLE pSession, CHAR *pHeaderBuffer, UINT32 *nLength);
/*************************************************************************** 
* Function: HTTPClientFindCloseHeader 
* Description: Terminate a headers search session. 
* 
* Input: pSession: HTTP Session handle. 
* 
* Output: None 
* 
* Return: HTTP Status 
* 
* Date : 2014-6-6 
****************************************************************************/ 
UINT32                  HTTPClientFindCloseHeader     (HTTP_SESSION_HANDLE pSession);

#if TLS_CONFIG_HTTP_CLIENT_TASK
typedef void  (*http_client_recv_callback_fn)(HTTP_SESSION_HANDLE pSession, CHAR * data, u32 datalen);
typedef void  (*http_client_err_callback_fn)(HTTP_SESSION_HANDLE pSession, int err);

typedef struct _http_client_msg
{
    HTTP_SESSION_HANDLE pSession;
    HTTPParameters param;
    HTTP_VERB method;
    CHAR* sendData;
    u32      dataLen;
    http_client_recv_callback_fn recv_fn;
    http_client_err_callback_fn err_fn;
} http_client_msg;

int http_client_task_init(void);
int http_client_post(http_client_msg * msg);
#endif //TLS_CONFIG_HTTP_CLIENT_TASK

#endif //WM_HTTP_CLIENT_H
