
#ifndef HTTP_CLIENT_WRAPPER
#define HTTP_CLIENT_WRAPPER

#include "am_openat_httpclient.h"
#if TLS_CONFIG_HTTP_CLIENT_SECURE
#include "matrixsslApi.h"
#endif
// Compilation mode
#define _HTTP_BUILD_WIN32            // Set Windows Build flag


///////////////////////////////////////////////////////////////////////////////
//
// Section      : Microsoft Windows Support
// Last updated : 01/09/2005
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _HTTP_BUILD_WIN32
#if 0
//#pragma warning (disable: 4996) // 'function': was declared deprecated (VS 2005)
#include <stdlib.h>
#include <string.h>
//#include <memory.h>
#include <stdio.h>
#include <ctype.h>
#endif
//#include <time.h>
//#include <winsock.h>

// Generic types
// Sockets (Winsock wrapper)


#define                              HTTP_ECONNRESET     (OPENAT_ECONNRESET) 
//#define                              HTTP_EINPROGRESS    (OPENAT_EINPROGRESS)
#define                              HTTP_EWOULDBLOCK    (OPENAT_EWOULDBLOCK)
#define                              HTTP_ETIMEOUT       (OPENAT_ETIMEDOUT)


//#define SOCKET_ERROR            (-1)
#define SOCKET_SSL_MORE_DATA            (-2)
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Section      : Functions that are not supported by the AMT stdc framework
//                So they had to be specificaly added.
// Last updated : 01/09/2005
//
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus 
extern "C" { 
#endif

    // STDC Wrapper implimentation
    int                                 HTTPWrapperIsAscii              (int c);
    int                                 HTTPWrapperToUpper              (int c);
    int                                 HTTPWrapperToLower              (int c);
    int                                 HTTPWrapperIsAlpha              (int c);
    int                                 HTTPWrapperIsAlNum              (int c);
    char*                               HTTPWrapperItoa                 (char *buff,int i);
    void                                HTTPWrapperInitRandomeNumber    (void);
    long                                HTTPWrapperGetUpTime            (void);
    int                                 HTTPWrapperGetRandomeNumber     (void);
    int                                 HTTPWrapperGetSocketError       (int s);
    unsigned long                       HTTPWrapperGetHostByName        (char *name,unsigned long *address);
    int                                 HTTPWrapperShutDown             (int s,int in);  
    // SSL Wrapper prototypes
#if TLS_CONFIG_HTTP_CLIENT_SECURE
    int                                 HTTPWrapperSSLConnect           (ssl_t **ssl_p,int s,const struct sockaddr *name,int namelen,char *hostname);
    int                                 HTTPWrapperSSLNegotiate         (HTTP_SESSION_HANDLE pSession,int s,const struct sockaddr *name,int namelen,char *hostname);
    int                                 HTTPWrapperSSLSend              (ssl_t *ssl,int s,char *buf, int len,int flags);
    int                                 HTTPWrapperSSLRecv              (ssl_t *ssl,int s,char *buf, int len,int flags);
    int                                 HTTPWrapperSSLClose             (ssl_t *ssl, int s);
    int                                 HTTPWrapperSSLRecvPending       (HTTP_SESSION_HANDLE pSession,int s);
#endif
    // Global wrapper Functions
#define                             IToA                            HTTPWrapperItoa
#define                             GetUpTime                       HTTPWrapperGetUpTime
#define                             SocketGetErr                    HTTPWrapperGetSocketError 
#define                             HostByName                      HTTPWrapperGetHostByName
#define                             InitRandomeNumber               HTTPWrapperInitRandomeNumber
#define                             GetRandomeNumber                HTTPWrapperGetRandomeNumber

#ifdef __cplusplus 
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Section      : Global type definitions
// Last updated : 01/09/2005
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NULL
#define NULL                         0
#endif

// Global socket structures and definitions
#define                              HTTP_INVALID_SOCKET (-1)
typedef struct sockaddr_in           HTTP_SOCKADDR_IN;
typedef struct timeval               HTTP_TIMEVAL; 
typedef struct hostent               HTTP_HOSTNET;
typedef struct sockaddr              HTTP_SOCKADDR;
typedef struct in_addr               HTTP_INADDR;


#endif // HTTP_CLIENT_WRAPPER
