#ifndef _TLS_CLIENT_H_
#define _TLS_CLIENT_H_
#include <mbedtls/net.h>
#include <mbedtls/ssl.h>
#include <mbedtls/havege.h>
#include "net_sockets.h"
#include "entropy.h"
#include "ctr_drbg.h"


typedef enum{
	MBEDTLS_ERR_TLS_ERR_NO,
	MBEDTLS_ERR_TLS_ERR_MALLOC,
	MBEDTLS_ERR_TLS_ERR_CA_ERR,
	MBEDTLS_ERR_TLS_ERR_CONFIG_ERR,
	MBEDTLS_ERR_TLS_ERR_CONN_ERR,
	MBEDTLS_ERR_TLS_ERR_HANDSHAKE_ERR,
	MBEDTLS_ERR_TLS_ERR_SEND,
}E_MBEDTLS_ERR_TLS;


#define DEBUG_LEVEL 3   /*debug µÈ¼¶*/

typedef struct{
	mbedtls_net_context client_fd;
	mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
}MBEDTLS_TLS_HANDLE;




#endif
