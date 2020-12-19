/******************************************************************************
 * TLS
 *****************************************************************************/
#include <stdio.h>
#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "iot_fs.h"
#include "iot_flash.h"
#include "iot_pmd.h"
#include "net_sockets.h"
#include "entropy.h"
#include "ssl.h"
#include "ctr_drbg.h"
#include "entropy.h"
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
//#include <fcntl.h>
//#include <netdb.h>
#include <errno.h>

#ifndef IPPROTO_TCP
#define IPPROTO_TCP     6
#endif

#define read(fd,buf,len)        recv( fd, (char*)( buf ), (int)( len ), 0 )
#define write(fd,buf,len)       send( fd, (char*)( buf ), (int)( len ), 0 )
#define close(fd)               close(fd)


// TLS operations
#define mbedtls_printf(fmt,args...) OPENAT_print("[ssl]"fmt ,##args)
HANDLE hMbedtlsTask;
E_OPENAT_NETWORK_STATE g_network_state = OPENAT_NETWORK_DISCONNECT;


typedef enum{
	MBEDTLS_ERR_TLS_ERR_NO,
	MBEDTLS_ERR_TLS_ERR_MALLOC,
	MBEDTLS_ERR_TLS_ERR_CA_ERR,
	MBEDTLS_ERR_TLS_ERR_CONFIG_ERR,
	MBEDTLS_ERR_TLS_ERR_CONN_ERR,
	MBEDTLS_ERR_TLS_ERR_HANDSHAKE_ERR,
	MBEDTLS_ERR_TLS_ERR_SEND,
}E_MBEDTLS_ERR_TLS;


#define DEBUG_LEVEL 3   /*debug 碌脠录露*/

typedef struct{
	mbedtls_net_context client_fd;
	mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
}MBEDTLS_TLS_HANDLE;


void wxpp_tls_debug(void *tag, int ret, const char* file, int line, const char *str)
{
	OPENAT_print("[WXPP] ret=%d,func=%s,line=%d,str=%s",ret,file,line,str);
}

 /*
 * Initiate a TCP connection with host:port and the given protocol
 */
int mbedtls_net_connect_v2( mbedtls_net_context *ctx, const char *host,
                         unsigned short port, int proto )
{
    int ret = -1;
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(struct sockaddr_in));
  	sa.sin_family = AF_INET;
	
	sa.sin_addr.s_addr = ipaddr_addr(host);
  	if (sa.sin_addr.s_addr == OPENAT_INADDR_NONE)
    {
		struct hostent *hp = gethostbyname(host);
		if (!hp || !hp->h_addr)
			return MBEDTLS_ERR_NET_UNKNOWN_HOST;
      	sa.sin_addr = *(struct in_addr *)hp->h_addr;
    }
	sa.sin_port = ((port & 0xff) << 8) | ((port & 0xff00) >> 8);
	ctx->fd = socket(OPENAT_AF_INET, OPENAT_SOCK_STREAM, 0);
    if (ctx->fd < 0)
    {
    	ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
        return ret;
    }
    // 建立TCP链接
    //memset(&TCPServerAddr, 0, sizeof(TCPServerAddr)); // 初始化服务器地址
    //TCPServerAddr.sin_family = AF_INET;
    //TCPServerAddr.sin_port = htons((unsigned short)port);
    if ((ret = connect(ctx->fd, (struct sockaddr *)&sa, sizeof(struct sockaddr))) < 0)
    {
        close(ctx->fd);
    }
	return ret;
}

 /*
 * Gracefully close the connection
 */
void mbedtls_net_free( mbedtls_net_context *ctx )
{
    if( ctx->fd == -1 )
        return;

    lwip_shutdown( ctx->fd, 2 );
    close( ctx->fd );

    ctx->fd = -1;
}

 /*
 * Initialize a context
 */
void mbedtls_net_init( mbedtls_net_context *ctx )
{
    ctx->fd = -1;
}
 /*
 * Check if the requested operation would be blocking on a non-blocking socket
 * and thus 'failed' with a negative return value.
 *
 * Note: on a blocking socket this function always returns 0!
 */
static int net_would_block( const mbedtls_net_context *ctx )
{
    int err = errno;

    /*
     * Never return 'WOULD BLOCK' on a non-blocking socket
     */
    if( ( fcntl( ctx->fd, F_GETFL, 0 ) & O_NONBLOCK ) != O_NONBLOCK )
    {
        errno = err;
        return( 0 );
    }

    switch( errno = err )
    {
#if defined EAGAIN
        case EAGAIN:
#endif
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
            return( 1 );
    }
    return( 0 );
}

 /*
 * Read at most 'len' characters
 */
int mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len )
{
    int ret;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if( fd < 0 )
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );

    ret = (int) read( fd, buf, len );

    if( ret < 0 )
    {
        if( net_would_block( ctx ) != 0 )
            return( MBEDTLS_ERR_SSL_WANT_READ );

        if( errno == EPIPE || errno == ECONNRESET )
            return( MBEDTLS_ERR_NET_CONN_RESET );

        if( errno == EINTR )
            return( MBEDTLS_ERR_SSL_WANT_READ );

        return( MBEDTLS_ERR_NET_RECV_FAILED );
    }

    return( ret );
}

 /*
 * Read at most 'len' characters, blocking for at most 'timeout' ms
 */
int mbedtls_net_recv_timeout( void *ctx, unsigned char *buf,
                              size_t len, uint32_t timeout )
{
    int ret;
    struct timeval tv;
    fd_set read_fds;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if( fd < 0 )
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );

    FD_ZERO( &read_fds );
    FD_SET( fd, &read_fds );

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = ( timeout % 1000 ) * 1000;

    ret = select( fd + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv );

    /* Zero fds ready means we timed out */
    if( ret == 0 )
        return( MBEDTLS_ERR_SSL_TIMEOUT );

    if( ret < 0 )
    {

        if( errno == EINTR )
            return( MBEDTLS_ERR_SSL_WANT_READ );

        return( MBEDTLS_ERR_NET_RECV_FAILED );
    }

    /* This call will not block */
    return( mbedtls_net_recv( ctx, buf, len ) );
}

 /*
 * Write at most 'len' characters
 */
int mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len )
{
    int ret;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if( fd < 0 )
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );

    ret = (int) write( fd, buf, len );

    if( ret < 0 )
    {
        if( net_would_block( ctx ) != 0 )
            return( MBEDTLS_ERR_SSL_WANT_WRITE );


        if( errno == EPIPE || errno == ECONNRESET )
            return( MBEDTLS_ERR_NET_CONN_RESET );

        if( errno == EINTR )
            return( MBEDTLS_ERR_SSL_WANT_WRITE );


        return( MBEDTLS_ERR_NET_SEND_FAILED );
    }

    return( ret );
}  


static void demo_mbedtls_free_context(MBEDTLS_TLS_HANDLE *handle)
{
	
	mbedtls_net_free( &handle->client_fd );

    mbedtls_x509_crt_free( &handle->cacert );
    mbedtls_ssl_free( &handle->ssl );
    mbedtls_ssl_config_free( &handle->conf );
    mbedtls_ctr_drbg_free( &handle->ctr_drbg );
    mbedtls_entropy_free( &handle->entropy );
	free(handle);
	
}


static int demo_mbedtls_init(void **handle, const char *host, unsigned short port, const char *ca_crt,
                     unsigned int ca_crt_len)
{
	int ret = 1, len;	
	E_MBEDTLS_ERR_TLS exit_code = MBEDTLS_ERR_TLS_ERR_NO;
	const char *pers = "ssl_client";
	*handle = (void *)malloc(sizeof(MBEDTLS_TLS_HANDLE));
	
	MBEDTLS_TLS_HANDLE *wxpp_handle = (MBEDTLS_TLS_HANDLE *)*handle;
	memset(wxpp_handle,0,sizeof(MBEDTLS_TLS_HANDLE));
	mbedtls_net_context *client_fd = &wxpp_handle->client_fd;	
	mbedtls_entropy_context *entropy = &wxpp_handle->entropy;
	mbedtls_ctr_drbg_context *ctr_drbg = &wxpp_handle->ctr_drbg;
	mbedtls_ssl_context *ssl = &wxpp_handle->ssl;
	mbedtls_ssl_config *conf = &wxpp_handle->conf;
	mbedtls_x509_crt *cacert = &wxpp_handle->cacert;

	
	/*
	* 0. Initialize the RNG and the session data
	*/

	mbedtls_net_init( client_fd );
	mbedtls_ssl_init( ssl );
	mbedtls_ssl_config_init( conf );
	mbedtls_x509_crt_init( cacert );
	mbedtls_ctr_drbg_init( ctr_drbg );
	mbedtls_ssl_conf_dbg(conf,wxpp_tls_debug,NULL);

	
#if defined(MBEDTLS_DEBUG_C)
	mbedtls_debug_set_threshold( DEBUG_LEVEL );
#endif
	mbedtls_printf( "\n  . Seeding the random number generator..." );



	mbedtls_entropy_init( entropy );
	if( ( ret = mbedtls_ctr_drbg_seed( ctr_drbg, mbedtls_entropy_func, entropy,
							   (const unsigned char *) pers,
							   strlen( pers ) ) ) != 0 )
	{
		mbedtls_printf( " failed\n	! mbedtls_ctr_drbg_seed returned %d\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONFIG_ERR; 
		goto exit;
	}

	mbedtls_printf( " ok\n" );
	  /*
	 * 0. Initialize certificates
	 */
	mbedtls_printf( "  . Loading the CA root certificate ..." );

	if(ca_crt != NULL)
	{
		ret = mbedtls_x509_crt_parse( cacert, (const unsigned char *) ca_crt,
							  ca_crt_len+1);
		if( ret < 0 )
		{
			mbedtls_printf( " failed\n	!  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
			exit_code = MBEDTLS_ERR_TLS_ERR_CA_ERR;
			goto exit;
		}
	}
	mbedtls_printf( " ok (%d skipped)\n", ret );

	/*
	 * 2. Setup stuff
	 */
	mbedtls_printf( "  . Setting up the SSL/TLS structure..." );

	if( ( ret = mbedtls_ssl_config_defaults( conf,
					MBEDTLS_SSL_IS_CLIENT,
					MBEDTLS_SSL_TRANSPORT_STREAM,
					MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
	{
		mbedtls_printf( " failed\n	! mbedtls_ssl_config_defaults returned %d\n\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONFIG_ERR; 
		goto exit;
	}

	mbedtls_printf( " ok\n" );

	/* OPTIONAL is not optimal for security,
	 * but makes interop easier in this simplified example */
	mbedtls_ssl_conf_authmode( conf, MBEDTLS_SSL_VERIFY_NONE );
	mbedtls_ssl_conf_ca_chain( conf, cacert, NULL );
	mbedtls_ssl_conf_rng( conf, mbedtls_ctr_drbg_random, ctr_drbg );

	if( ( ret = mbedtls_ssl_setup( ssl, conf ) ) != 0 )
	{
		mbedtls_printf( " failed\n	! mbedtls_ssl_setup returned %d\n\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONFIG_ERR; 
		goto exit;
	}

	if( ( ret = mbedtls_ssl_set_hostname( ssl, host ) ) != 0 )
	{
		mbedtls_printf( " failed\n	! mbedtls_ssl_set_hostname returned %d\n\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONFIG_ERR; 
		goto exit;
	}
	mbedtls_printf( " ok\n" );
	
	mbedtls_ssl_set_bio( ssl, client_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout );

	return (int)exit_code;
	exit:

	demo_mbedtls_free_context(wxpp_handle);
	return (int)exit_code;
}

/**
 * @brief make a TLS connection
 *
 * @param handle        TLS connection handle
 * @param host          server host
 * @param port          server port
 * @param ca_crt        CA certificate in PEM format
 * @param ca_crt_len    CA certificate length
 * @param timeout_ms    timeout in millisecond
 *
 * @return 0 if success, or MBEDTLS_ERR_TLS_xxx if error
 */
int demo_mbedtls_connect(void **handle, const char *host, unsigned short port, const char *ca_crt,
                     unsigned int ca_crt_len, unsigned int timeout_ms)
{
    int ret = 1, len;
    E_MBEDTLS_ERR_TLS exit_code = MBEDTLS_ERR_TLS_ERR_NO;
	
	
    uint32_t flags;
	mbedtls_printf(" . tls init ....");
	if(exit_code = demo_mbedtls_init(handle,host,port,ca_crt,ca_crt_len)  != MBEDTLS_ERR_TLS_ERR_NO)
		return exit_code;

	MBEDTLS_TLS_HANDLE *wxpp_handle = (MBEDTLS_TLS_HANDLE*)*handle;
	/*
     * 1. Start the connection
     */
    mbedtls_printf( "  . Connecting to tcp/%s/%d...", host, port );

    if( ( ret = mbedtls_net_connect_v2( &wxpp_handle->client_fd, host,
                                         port, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_net_connect returned %d\n\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONN_ERR; 
        goto exit;
    }
	mbedtls_printf( " ok\n" );
    /*
     * 4. Handshake
     */
    mbedtls_printf( "  . Performing the SSL/TLS handshake..." );

    while( ( ret = mbedtls_ssl_handshake( &wxpp_handle->ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret );
			exit_code = MBEDTLS_ERR_TLS_ERR_HANDSHAKE_ERR; 
            goto exit;
        }
    }

    mbedtls_printf( " ok\n" );

    /*
     * 5. Verify the server certificate
     */
    mbedtls_printf( "  . Verifying peer X.509 certificate..." );

    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( &wxpp_handle->ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        mbedtls_printf( " failed\n" );

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        mbedtls_printf( "%s\n", vrfy_buf );
    }
    else
        mbedtls_printf( " ok\n" );


    return (int)exit_code;

exit:

    demo_mbedtls_free_context(wxpp_handle);

    return (int)exit_code;

}

/**
 * @brief disconnect a TLS connection
 * @param handle TLS connection handle
 */
void demo_mbedtls_disconnect(void *handle)
{
	MBEDTLS_TLS_HANDLE *wxpp_hand = (MBEDTLS_TLS_HANDLE *)handle;
	mbedtls_net_free( &wxpp_hand->client_fd );
    mbedtls_x509_crt_free( &wxpp_hand->cacert );
    mbedtls_ssl_free( &wxpp_hand->ssl );
    mbedtls_ssl_config_free( &wxpp_hand->conf );
    mbedtls_ctr_drbg_free( &wxpp_hand->ctr_drbg );
    mbedtls_entropy_free( &wxpp_hand->entropy );
	free(wxpp_hand);
}
/**
 * @brief write to a TLS connection
 *
 * @param handle        TLS connection handle
 * @param data          data to write
 * @param data_len      data size to write
 * @param timeout_ms    write timeout in millisecond
 * @param written_len   bytes written
 *
 * @return 0 if success, or MBEDTLS_ERR_TLS_xxx if error

 */
int demo_mbedtls_write(void *handle, const void *data, size_t data_len, unsigned int timeout_ms,
                   size_t *written_len)
{
	MBEDTLS_TLS_HANDLE *wxpp_hand = (MBEDTLS_TLS_HANDLE *)handle;
	*written_len = mbedtls_ssl_write( (void*)&wxpp_hand->ssl, data, data_len );
	if(*written_len > 0)
	{
		return MBEDTLS_ERR_TLS_ERR_NO;
	}
	else
	{
		return MBEDTLS_ERR_TLS_ERR_SEND;
	}
}
/**
 * @brief block read from TLS connection
 *
 * @param handle        TLS connection handle
 * @param buffer        output buffer
 * @param buffer_len    output buffer size
 * @param timeout_ms    read timeout in millisecond
 * @param read_len      bytes read
 *
 * @return 0 if success, or MBEDTLS_ERR_TLS_xxx if error
 */
int demo_mbedtls_read(void *handle, void *buffer, size_t buffer_len, unsigned int timeout_ms,
                  size_t *read_len)
{
 	MBEDTLS_TLS_HANDLE *wxpp_hand = (MBEDTLS_TLS_HANDLE *)handle;
	*read_len = mbedtls_ssl_read((void*)&wxpp_hand->ssl, buffer,
                              buffer_len);
	if(*read_len > 0)
	{
		return MBEDTLS_ERR_TLS_ERR_NO;
	}
	else
	{
		return MBEDTLS_ERR_TLS_ERR_SEND;
	}
}


void demo_mbedtls_test_task(void *ctx)
{
	void *wxpp_handle = NULL;
	const char *data = "GET / HTTP/1.1\r\nHost: 36.7.87.100\r\nConnection: keep-alive\r\n\r\n\x1a";
	size_t write_len;
	size_t read_len;
	char *read_buf = malloc(1024);
	while(1)
	{
		if(g_network_state == OPENAT_NETWORK_LINKED)
		{
			break;
		}
		osiThreadSleep(1000);
	}
	//osiThreadSleep(10*1000); 

	if(demo_mbedtls_connect(&wxpp_handle,"36.7.87.100",4433,NULL,0,0) == 0)
	{
		mbedtls_printf("LINK SERVERS OK");
		if(demo_mbedtls_write(wxpp_handle,data,strlen(data),0,&write_len) != MBEDTLS_ERR_TLS_ERR_NO)
		{
			mbedtls_printf(" wxpp_tls_write err");
			free(read_buf);
			demo_mbedtls_free_context((MBEDTLS_TLS_HANDLE*)wxpp_handle);
		}
		mbedtls_printf("wxpp_tls_write len = %d",write_len);
		
		for(;;)
		{
			memset(read_buf,0,1024);
			if(demo_mbedtls_read(wxpp_handle,read_buf,1024,100,&read_len) != MBEDTLS_ERR_TLS_ERR_NO)
			{
				mbedtls_printf("wxpp_tls_read err read_len = %d",read_len);	
				free(read_buf);
				demo_mbedtls_free_context((MBEDTLS_TLS_HANDLE*)wxpp_handle);
				break;
			}

			mbedtls_printf("wxpp_tls_read len = %d",read_len);
			mbedtls_printf("wxpp_tls_read buff = %s",read_buf);

		}
	}
	else
	{
		mbedtls_printf("LINK SERVERS ERROR");
	}

	mbedtls_printf("demo_mbedtls_test_task over");

	osiThreadSleep(1000);
	//osiThreadExit();
}


void mbedtls_test_init()
{
	#if 0
	hMbedtlsTask = iot_os_create_task(wxpp_test_task,
                    NULL,
                    20*1024,
                    24,
                    OPENAT_OS_CREATE_DEFAULT,
                    "demo_socket_mbedtls");
	#endif
    hMbedtlsTask = iot_os_create_task(demo_mbedtls_test_task,
                  NULL,
                  20*1024,
                  5,
                  OPENAT_OS_CREATE_DEFAULT,
                  "demo_mbedtls");
}

static void SSL_NetworkIndCallBack(E_OPENAT_NETWORK_STATE state)
{

    iot_debug_print("[ssl] network ind state %d", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
    	g_network_state = OPENAT_NETWORK_LINKED;
    }
    else if(state == OPENAT_NETWORK_READY)
    {
		g_network_state = OPENAT_NETWORK_READY;
    }
}


int appimg_enter(void *param)
{    
	iot_debug_print("[ssl] appimg_enter");
    iot_network_set_cb(SSL_NetworkIndCallBack);
	iot_pmd_exit_deepsleep();
	mbedtls_test_init();

    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[ssl] appimg_exit");
}

