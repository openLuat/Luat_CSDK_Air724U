//#include <stdio.h>
#include "string.h"
#include "iot_os.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "iot_fs.h"
#include "iot_flash.h"
#include "iot_pmd.h"
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
//#include <fcntl.h>
//#include <netdb.h>
#include <errno.h>
#include "tls_client.h"
#ifndef IPPROTO_TCP
#define IPPROTO_TCP     6
#endif

#define mbedtls_printf(fmt,args...) OPENAT_print("[ssl]"fmt ,##args)

void wxpp_tls_debug(void *tag, int ret, const char* file, int line, const char *str)
{
	OPENAT_print("[WXPP] ret=%d,func=%s,line=%d,str=%s",ret,file,line,str);
}

 /*
 * Gracefully close the connection
 */
void mbedtls_net_free( mbedtls_net_context *ctx )
{
    if( ctx->fd == -1 )
        return;

    lwip_shutdown( ctx->fd, 2 );
    lwip_close( ctx->fd );

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
    if( ( lwip_fcntl( ctx->fd, F_GETFL, 0 ) & O_NONBLOCK ) != O_NONBLOCK )
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

    ret = (int) lwip_read( fd, buf, len );

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

    ret = lwip_select( fd + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv );

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

    ret = (int) lwip_write( fd, buf, len );

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


static void rtmp_mbedtls_free_context(MBEDTLS_TLS_HANDLE *handle)
{
	
	mbedtls_net_free( &handle->client_fd );

    mbedtls_x509_crt_free( &handle->cacert );
    mbedtls_ssl_free( &handle->ssl );
    mbedtls_ssl_config_free( &handle->conf );
    mbedtls_ctr_drbg_free( &handle->ctr_drbg );
    mbedtls_entropy_free( &handle->entropy );
	free(handle);
	
}

int rtmp_mbedtls_set_fd(MBEDTLS_TLS_HANDLE *handle,int fd)
{
	handle->client_fd.fd = fd;
}

int rtmp_mbedtls_init(MBEDTLS_TLS_HANDLE *handle)
{
	int ret = 1, len;	
	E_MBEDTLS_ERR_TLS exit_code = MBEDTLS_ERR_TLS_ERR_NO;
	const char *pers = "ssl_client";
	
	memset(handle,0,sizeof(MBEDTLS_TLS_HANDLE));
	mbedtls_net_context *client_fd = &handle->client_fd;	
	mbedtls_entropy_context *entropy = &handle->entropy;
	mbedtls_ctr_drbg_context *ctr_drbg = &handle->ctr_drbg;
	mbedtls_ssl_context *ssl = &handle->ssl;
	mbedtls_ssl_config *conf = &handle->conf;
	mbedtls_x509_crt *cacert = &handle->cacert;

	
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

	/*没证书，注释掉*/
	#if 0
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
	#endif
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

	mbedtls_printf( " ok\n" );
	
	mbedtls_ssl_set_bio( ssl, client_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout );

	return (int)exit_code;
	exit:

	rtmp_mbedtls_free_context(handle);
	return (int)exit_code;
}


int rtmp_mbedtls_handshake(MBEDTLS_TLS_HANDLE *handle)
{
    int ret = 1, len;
    E_MBEDTLS_ERR_TLS exit_code = MBEDTLS_ERR_TLS_ERR_NO;
	
	
    uint32_t flags;
	/*上层已经做过，注释掉*/
	#if 0
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
	#endif
    while( ( ret = mbedtls_ssl_handshake( &handle->ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret );
			exit_code = MBEDTLS_ERR_TLS_ERR_HANDSHAKE_ERR; 
            goto exit;
        }
    }
	
	/*不需要校验服务器证书，注释掉*/
	#if 0
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

	#endif
    return (int)exit_code;

exit:

    rtmp_mbedtls_free_context(handle);

    return (int)exit_code;

}

/**
 * @brief disconnect a TLS connection
 * @param handle TLS connection handle
 */
void rtmp_mbedtls_disconnect(void *handle)
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

int rtmp_mbedtls_write(void *handle, const void *data, size_t data_len)
{
	MBEDTLS_TLS_HANDLE *wxpp_hand = (MBEDTLS_TLS_HANDLE *)handle;
	return mbedtls_ssl_write( (void*)&wxpp_hand->ssl, data, data_len );

}


int rtmp_mbedtls_read(void *handle, void *buffer, size_t buffer_len)
{
 	MBEDTLS_TLS_HANDLE *wxpp_hand = (MBEDTLS_TLS_HANDLE *)handle;
	return mbedtls_ssl_read((void*)&wxpp_hand->ssl, buffer,
                              buffer_len);
}

