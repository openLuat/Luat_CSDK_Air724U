#ifndef __RTMP_SYS_H__
#define __RTMP_SYS_H__
/*
 *      Copyright (C) 2010 Howard Chu
 *
 *  This file is part of librtmp.
 *
 *  librtmp is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1,
 *  or (at your option) any later version.
 *
 *  librtmp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with librtmp see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *  http://www.gnu.org/copyleft/lgpl.html
 */

#ifdef _WIN32

#ifdef _XBOX
#include <xtl.h>
#include <winsockx.h>
#define snprintf _snprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define vsnprintf _vsnprintf

#else /* !_XBOX */
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#define GetSockError()	WSAGetLastError()
#define SetSockError(e)	WSASetLastError(e)
#define setsockopt(a,b,c,d,e)	(setsockopt)(a,b,c,(const char *)d,(int)e)
#define EWOULDBLOCK	WSAETIMEDOUT	/* we don't use nonblocking, but we do use timeouts */
#define sleep(n)	Sleep(n*1000)
#define msleep(n)	Sleep(n)
#define SET_RCVTIMEO(tv,s)	int tv = s*1000
#else /* !_WIN32 */
#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "sys/time.h"
#include <assert.h>

#define GetSockError()	errno
#define SetSockError(e)	errno = e
#undef closesocket
#define closesocket(s)	lwip_close(s)
#define msleep(n)	iot_os_sleep(n*1000)
#define SET_RCVTIMEO(tv,s)	struct timeval tv = {s,0}
#endif

#include "rtmp.h"
#define INADDR_NONE             0xffffffff
#define INADDR_ANY              (u_long)0x00000000

#ifdef USE_POLARSSL


#include "tls_client.h"
//#include <mbedtls/Compat-1.3.h>
typedef struct tls_ctx {
	mbedtls_havege_state hs;
	mbedtls_ssl_session ssn;
} tls_ctx;

typedef struct 
{
	mbedtls_net_context client_fd;
	mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
}tls_context;


#define TLS_CTX tls_ctx *
#define TLS_client(ctx, s)	s = malloc(sizeof(MBEDTLS_TLS_HANDLE));  rtmp_mbedtls_init((MBEDTLS_TLS_HANDLE*)s)
			
#define TLS_setfd(s,fd)	  rtmp_mbedtls_set_fd((MBEDTLS_TLS_HANDLE*)s,fd);

#define TLS_connect(s)	rtmp_mbedtls_handshake((MBEDTLS_TLS_HANDLE*)s)

#define TLS_read(s,b,l)	rtmp_mbedtls_read((MBEDTLS_TLS_HANDLE*)s,(unsigned char *)b,l)

#define TLS_write(s,b,l)	rtmp_mbedtls_write((MBEDTLS_TLS_HANDLE*)s,(unsigned char *)b,l)

#define TLS_shutdown(s)	 //ssl_close_notify(s)

#define TLS_close(s)	rtmp_mbedtls_disconnect((MBEDTLS_TLS_HANDLE*)s);

#elif defined(USE_GNUTLS)
#include <gnutls/gnutls.h>
typedef struct tls_ctx {
	gnutls_certificate_credentials_t cred;
	gnutls_priority_t prios;
} tls_ctx;
#define TLS_CTX	tls_ctx *
#define TLS_client(ctx,s)	gnutls_init((gnutls_session_t *)(&s), GNUTLS_CLIENT); gnutls_priority_set(s, ctx->prios); gnutls_credentials_set(s, GNUTLS_CRD_CERTIFICATE, ctx->cred)
#define TLS_setfd(s,fd)	gnutls_transport_set_ptr(s, (gnutls_transport_ptr_t)(long)fd)
#define TLS_connect(s)	gnutls_handshake(s)
#define TLS_read(s,b,l)	gnutls_record_recv(s,b,l)
#define TLS_write(s,b,l)	gnutls_record_send(s,b,l)
#define TLS_shutdown(s)	gnutls_bye(s, GNUTLS_SHUT_RDWR)
#define TLS_close(s)	gnutls_deinit(s)

#elif defined(USE_OPENSSL)	/* USE_OPENSSL */
#define TLS_CTX	SSL_CTX *
#define TLS_client(ctx,s)	s = SSL_new(ctx)
#define TLS_setfd(s,fd)	SSL_set_fd(s,fd)
#define TLS_connect(s)	SSL_connect(s)
#define TLS_read(s,b,l)	SSL_read(s,b,l)
#define TLS_write(s,b,l)	SSL_write(s,b,l)
#define TLS_shutdown(s)	SSL_shutdown(s)
#define TLS_close(s)	SSL_free(s)

#endif
#endif
