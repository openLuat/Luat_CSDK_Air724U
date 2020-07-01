/*
 * 通过连接101.132.154.251 测试双向认证的SSL握手过程，及数据加密收发，该服务器随时会关闭，自己测试时必须是使用自己的服务器
 */
#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "iot_fs.h"
#include "iot_flash.h"
#include "iot_types.h"
#include "iot_pmd.h"
#include "ssllib.h"

typedef struct {
    UINT8 type;
    UINT8 data;
}DEMO_SSL_MESSAGE;


#define SSL_MSG_NETWORK_READY (0)
#define SSL_MSG_NETWORK_LINKED (1)
#define SSL_MSG_USER_MSG_TIMER (2)



#define DBG_INFO(X, Y...)	iot_debug_print("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
#define DBG_ERROR(X, Y...)	iot_debug_print("%s %d:"X, __FUNCTION__, __LINE__, ##Y)

#define SOCKET_CLOSE(A)         if (A >= 0) {close(A);A = -1;}
#if 1
#define TEST_URL					"www.baidu.com"
#define TEST_DATA					"GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: keep-alive\r\n\r\n"
#define TEST_PORT					(443)
#else
#define TEST_IP						"180.101.49.11"
#define TEST_DATA					"GET / HTTP/1.1\r\nHost: 180.101.49.11\r\nConnection: keep-alive\r\n\r\n"
#define TEST_PORT					443
#endif

#define SSL_RECONNECT_MAX			(8)
#define SSL_HEAT_TO					20
static HANDLE hTimer;
static HANDLE hSocketTask;
static E_OPENAT_NETWORK_STATE NWState;				//网络状态
static uint8_t ToFlag = 0;

//次级证书
const char *SymantecClass3SecureServerCA_G4 = "-----BEGIN CERTIFICATE-----\r\n"
		"MIIEaTCCA1GgAwIBAgILBAAAAAABRE7wQkcwDQYJKoZIhvcNAQELBQAwVzELMAkG\r\n"
		"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n"
		"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw0xNDAyMjAxMDAw\r\n"
		"MDBaFw0yNDAyMjAxMDAwMDBaMGYxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n"
		"YWxTaWduIG52LXNhMTwwOgYDVQQDEzNHbG9iYWxTaWduIE9yZ2FuaXphdGlvbiBW\r\n"
		"YWxpZGF0aW9uIENBIC0gU0hBMjU2IC0gRzIwggEiMA0GCSqGSIb3DQEBAQUAA4IB\r\n"
		"DwAwggEKAoIBAQDHDmw/I5N/zHClnSDDDlM/fsBOwphJykfVI+8DNIV0yKMCLkZc\r\n"
		"C33JiJ1Pi/D4nGyMVTXbv/Kz6vvjVudKRtkTIso21ZvBqOOWQ5PyDLzm+ebomchj\r\n"
		"SHh/VzZpGhkdWtHUfcKc1H/hgBKueuqI6lfYygoKOhJJomIZeg0k9zfrtHOSewUj\r\n"
		"mxK1zusp36QUArkBpdSmnENkiN74fv7j9R7l/tyjqORmMdlMJekYuYlZCa7pnRxt\r\n"
		"Nw9KHjUgKOKv1CGLAcRFrW4rY6uSa2EKTSDtc7p8zv4WtdufgPDWi2zZCHlKT3hl\r\n"
		"2pK8vjX5s8T5J4BO/5ZS5gIg4Qdz6V0rvbLxAgMBAAGjggElMIIBITAOBgNVHQ8B\r\n"
		"Af8EBAMCAQYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUlt5h8b0cFilT\r\n"
		"HMDMfTuDAEDmGnwwRwYDVR0gBEAwPjA8BgRVHSAAMDQwMgYIKwYBBQUHAgEWJmh0\r\n"
		"dHBzOi8vd3d3Lmdsb2JhbHNpZ24uY29tL3JlcG9zaXRvcnkvMDMGA1UdHwQsMCow\r\n"
		"KKAmoCSGImh0dHA6Ly9jcmwuZ2xvYmFsc2lnbi5uZXQvcm9vdC5jcmwwPQYIKwYB\r\n"
		"BQUHAQEEMTAvMC0GCCsGAQUFBzABhiFodHRwOi8vb2NzcC5nbG9iYWxzaWduLmNv\r\n"
		"bS9yb290cjEwHwYDVR0jBBgwFoAUYHtmGkUNl8qJUC99BM00qP/8/UswDQYJKoZI\r\n"
		"hvcNAQELBQADggEBAEYq7l69rgFgNzERhnF0tkZJyBAW/i9iIxerH4f4gu3K3w4s\r\n"
		"32R1juUYcqeMOovJrKV3UPfvnqTgoI8UV6MqX+x+bRDmuo2wCId2Dkyy2VG7EQLy\r\n"
		"XN0cvfNVlg/UBsD84iOKJHDTu/B5GqdhcIOKrwbFINihY9Bsrk8y1658GEV1BSl3\r\n"
		"30JAZGSGvip2CTFvHST0mdCF/vIhCPnG9vHQWe3WVjwIKANnuvD58ZAWR65n5ryA\r\n"
		"SOlCdjSXVWkkDoPWoC209fN5ikkodBpBocLTJIg1MGCUF7ThBCIxPTsvFwayuJ2G\r\n"
		"K1pp74P1S8SqtCr4fKGxhZSM9AyHDPSsQPhZSZg=\r\n"
		"-----END CERTIFICATE-----";
//不是baidu使用的次级根证书，可以加载这个，验证程序的确去检查了证书链，注意，只加载这个证书会导致baidu的SSL握手失败
const char *SymantecClass3SecureServerSHA256SSLCA = "-----BEGIN CERTIFICATE-----\r\n"
		"MIIFSTCCBDGgAwIBAgIQaYeUGdnjYnB0nbvlncZoXjANBgkqhkiG9w0BAQsFADCB\r\n"
		"vTELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\r\n"
		"ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwOCBWZXJp\r\n"
		"U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MTgwNgYDVQQDEy9W\r\n"
		"ZXJpU2lnbiBVbml2ZXJzYWwgUm9vdCBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTAe\r\n"
		"Fw0xMzA0MDkwMDAwMDBaFw0yMzA0MDgyMzU5NTlaMIGEMQswCQYDVQQGEwJVUzEd\r\n"
		"MBsGA1UEChMUU3ltYW50ZWMgQ29ycG9yYXRpb24xHzAdBgNVBAsTFlN5bWFudGVj\r\n"
		"IFRydXN0IE5ldHdvcmsxNTAzBgNVBAMTLFN5bWFudGVjIENsYXNzIDMgU2VjdXJl\r\n"
		"IFNlcnZlciBTSEEyNTYgU1NMIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\r\n"
		"CgKCAQEAvjgWUYuA2+oOTezoP1zEfKJd7TuvpdaeEDUs48XlqN6Mhhcm5t4LUUos\r\n"
		"0PvRFFpy98nduIMcxkaMMSWRDlkXo9ATjJLBr4FUTrxiAp6qpxpX2MqmmXpwVk+Y\r\n"
		"By5LltBMOVO5YS87dnyOBZ6ZRNEDVHcpK1YqqmHkhC8SFTy914roCR5W8bUUrIqE\r\n"
		"zq54omAKU34TTBpAcA5SWf9aaC5MRhM7OQmCeAI1SSAIgrOxbIkPbh41JbAsJIPj\r\n"
		"xVAsukaQRYcNcv9dETjFkXbFLPsFKoKVoVlj49AmWM1nVjq633zS0jvY3hp6d+QM\r\n"
		"jAvrK8IisL1Vutm5VdEiesYCTj/DNQIDAQABo4IBejCCAXYwEgYDVR0TAQH/BAgw\r\n"
		"BgEB/wIBADA+BgNVHR8ENzA1MDOgMaAvhi1odHRwOi8vY3JsLndzLnN5bWFudGVj\r\n"
		"LmNvbS91bml2ZXJzYWwtcm9vdC5jcmwwDgYDVR0PAQH/BAQDAgEGMDcGCCsGAQUF\r\n"
		"BwEBBCswKTAnBggrBgEFBQcwAYYbaHR0cDovL29jc3Aud3Muc3ltYW50ZWMuY29t\r\n"
		"MGsGA1UdIARkMGIwYAYKYIZIAYb4RQEHNjBSMCYGCCsGAQUFBwIBFhpodHRwOi8v\r\n"
		"d3d3LnN5bWF1dGguY29tL2NwczAoBggrBgEFBQcCAjAcGhpodHRwOi8vd3d3LnN5\r\n"
		"bWF1dGguY29tL3JwYTAqBgNVHREEIzAhpB8wHTEbMBkGA1UEAxMSVmVyaVNpZ25N\r\n"
		"UEtJLTItMzczMB0GA1UdDgQWBBTbYiD7fQKJfNI7b8fkMmwFUh2tsTAfBgNVHSME\r\n"
		"GDAWgBS2d/ppSEefUxLVwuoHMnYH0ZcHGTANBgkqhkiG9w0BAQsFAAOCAQEAGcyV\r\n"
		"4i97SdBIkFP0B7EgRDVwFNVENzHv73DRLUzpLbBTkQFMVOd9m9o6/7fLFK0wD2ka\r\n"
		"KvC8zTXrSNy5h/3PsVr2Bdo8ZOYr5txzXprYDJvSl7Po+oeVU+GZrYjo+rwJTaLE\r\n"
		"ahsoOy3DIRXuFPqdmBDrnz7mJCRfehwFu5oxI1h5TOxtGBlNUR8IYb2RBQxanCb8\r\n"
		"C6UgJb9qGyv3AglyaYMyFMNgW379mjL6tJUOGvk7CaRUR5oMzjKv0SHMf9IG72AO\r\n"
		"Ym9vgRoXncjLKMziX24serTLR3x0aHtIcQKcIwnzWq5fQi5fK1ktUojljQuzqGH5\r\n"
		"S5tV1tqxkju/w5v5LA==\r\n"
		"-----END CERTIFICATE-----";

//根证书，测试用，如果使用自己的服务器，请自行修改
const char *RootCert = "-----BEGIN CERTIFICATE-----\r\n"
		"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n"
		"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n"
		"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n"
		"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n"
		"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n"
		"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n"
		"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n"
		"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n"
		"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n"
		"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n"
		"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n"
		"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n"
		"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n"
		"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n"
		"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n"
		"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n"
		"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n"
		"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n"
		"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n"
		"-----END CERTIFICATE-----";

const char *ClientCert = "-----BEGIN CERTIFICATE-----\r\n"
		"MIIDPjCCAiagAwIBAgIBBjANBgkqhkiG9w0BAQsFADBrMQswCQYDVQQGEwJjaDEL\r\n"
		"MAkGA1UECBMCemoxCzAJBgNVBAcTAmp4MQ4wDAYDVQQKEwVhZG1pbjEOMAwGA1UE\r\n"
		"CxMFYWRtaW4xDTALBgNVBAMTBHJvb3QxEzARBgkqhkiG9w0BCQEWBG5vbmUwHhcN\r\n"
		"MTcxMDE5MDYyODAwWhcNMTgxMDE5MDYyODAwWjBtMQswCQYDVQQGEwJjaDELMAkG\r\n"
		"A1UECBMCemoxCzAJBgNVBAcTAmp4MQ0wCwYDVQQKEwRsdWF0MQ0wCwYDVQQLEwRs\r\n"
		"dWF0MREwDwYDVQQDDAhzc2xfdGVzdDETMBEGCSqGSIb3DQEJARYEbm9uZTCBnzAN\r\n"
		"BgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAx6y1x3XuGa9B0KI9KZRMvjAkUKRV/HXM\r\n"
		"f2MhI6Q5EqyQIJbZBdfu7Tenobgggdncy0TT/eXZW8oTTM8cB+S4rGj4h98Osk7C\r\n"
		"XhYx/7Vd883jicfH+VJks1nvCNZI8bifSCJFHHtY4tNME8MbLxUu3DzRBYXzq2ZS\r\n"
		"e37aenI97EcCAwEAAaNvMG0wDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQUOn+eScMY\r\n"
		"BS2S8E1OYL/O/4+11ZAwCwYDVR0PBAQDAgSwMBEGCWCGSAGG+EIBAQQEAwIFoDAe\r\n"
		"BglghkgBhvhCAQ0EERYPeGNhIGNlcnRpZmljYXRlMA0GCSqGSIb3DQEBCwUAA4IB\r\n"
		"AQAlMHCy9FGRaF25TaHhEftbTe8iydq4/4xJUSLP3QcbDZxYDzYeBs9IgIv6BX24\r\n"
		"KIuxSwgwWNhTqEeapgI+pnImQCLGEjNq8Wn/JYXCrclqkMmQr1CHJiCEZtYBN/ou\r\n"
		"ky4wgEfTKUMqlRInZFrsQs9HFjINqXwz9Gg2PQeshPHVESolBBHohl831yuqMyQA\r\n"
		"YE0weAXFfp0VEdetVSetyVqCO6lb5XQZlozsKw6h5SUDw+uTYxBZdnyItOmhZdt3\r\n"
		"swZgYd3Dbg8KAI0P/PETf4xnMVR2NicbYc+zOuA68EUxwTQi6JkHeLaFNtAnltr8\r\n"
		"dedN8rjgY/b0z2MTzVY3OGLx\r\n"
		"-----END CERTIFICATE-----";

const char *ClientRSAKey = "-----BEGIN RSA PRIVATE KEY-----\r\n"
		"MIICXgIBAAKBgQDHrLXHde4Zr0HQoj0plEy+MCRQpFX8dcx/YyEjpDkSrJAgltkF\r\n"
		"1+7tN6ehuCCB2dzLRNP95dlbyhNMzxwH5LisaPiH3w6yTsJeFjH/tV3zzeOJx8f5\r\n"
		"UmSzWe8I1kjxuJ9IIkUce1ji00wTwxsvFS7cPNEFhfOrZlJ7ftp6cj3sRwIDAQAB\r\n"
		"AoGBAMRVFggh9RRcNyKl4+3WW/9F5u9EJygty/4VwqgA+f1an/zrVklgoRWu+60Q\r\n"
		"FyaWyXs1Gh00vBx8/a0wmCdKxilED3abjT6jbzoAKJYsjcRqthNAFlb6bNHdyQPO\r\n"
		"HZvuKsBS6ZHCeSoNYFuW4ncGCfEsvV/qRzYkAbr5CqVPxriBAkEA4n/lBp2ylgfb\r\n"
		"xK8WbGOXO+fPPAj8X7Ap+iTIjnespn0sIaMS1xMyQ5hXhJu7+BGsLDg6X8tWIWWt\r\n"
		"c7khvydkTwJBAOGuZlpxuJ+pU1Dlsd6W8fki3B1Mi+4U8dRiiq86lehw7vo0oI5U\r\n"
		"1NySbKqQDERL+SbRYL73a3CgBllq5TpNYokCQQC7BIE1ukY4DRsQRsWMD5tTEm+R\r\n"
		"kZXY6JtweKjEwdnjyl0DFSQ8RBRvrb0tuG03QlhYVsEUUc+3Wb4jXEyaCkuPAkAU\r\n"
		"aBatOvc8yKzV9c8dl3yN0I8ivxcwEgjD8Z0ktyFzATM6wKN7+0O8JilZSukxC8Wd\r\n"
		"svUSj4DRkEbCsx3DJdgxAkEAuBZ0Dmv3XYJ3zn/MAsNZzWLbbN+YPZ11nUTNE9FU\r\n"
		"M3paJdqmD70wz3tn5QhcIXbJ/90qs4iPNZ52qiOYnaMD3Q==\r\n"
		"-----END RSA PRIVATE KEY-----";
#ifdef TEST_URL
static uint32_t SSL_Gethostbyname(void)
{
    //域名解析
	openat_ip_addr_t *IP;
    struct openat_hostent *hostentP = NULL;
    char *ipAddr = NULL;

    //获取域名ip信息
    hostentP = gethostbyname(TEST_URL);

    if (!hostentP)
    {
        DBG_ERROR("gethostbyname %s fail", TEST_URL);
        return 0;
    }

    // 将ip转换成字符串
    ipAddr = ipaddr_ntoa((const openat_ip_addr_t *)hostentP->h_addr_list[0]);

    DBG_ERROR("gethostbyname %s ip %s", TEST_URL, ipAddr);
    IP = (openat_ip_addr_t *)hostentP->h_addr_list[0];
    return IP->addr;
}
#endif
static int32_t Socket_ConnectServer(void)
{
	iot_debug_print("[SSL] %s",__func__);
    int connErr;
    struct openat_sockaddr_in TCPServerAddr;
	int32_t Socketfd;
#ifdef TEST_URL
	uint32_t IP;
	IP = SSL_Gethostbyname();
#endif


#ifdef TEST_URL
	if (IP)
#else
	if (1)
#endif
	{
		Socketfd = socket(OPENAT_AF_INET,OPENAT_SOCK_STREAM,0);
	    if (Socketfd < 0)
	    {
	        DBG_ERROR("create tcp socket error");
	        return -1;
	    }
	    // 建立TCP链接
	    memset(&TCPServerAddr, 0, sizeof(TCPServerAddr)); // 初始化服务器地址
	    TCPServerAddr.sin_family = OPENAT_AF_INET;
	    TCPServerAddr.sin_port = htons((unsigned short)TEST_PORT);
#ifndef TEST_URL
	    inet_aton(TEST_IP, &TCPServerAddr.sin_addr);
#endif
#ifdef TEST_URL
	    TCPServerAddr.sin_addr.s_addr = IP;
#else
	    DBG_INFO("%08x", TCPServerAddr.sin_addr.s_addr);
#endif
	    connErr = connect(Socketfd, (const struct openat_sockaddr *)&TCPServerAddr, sizeof(struct openat_sockaddr));
	    if (connErr < 0)
	    {
	    	DBG_ERROR("tcp connect error %d", socket_errno(Socketfd));
	        close(Socketfd);
	        return -1;
	    }
	    DBG_INFO("[socket] tcp connect success");
	    return Socketfd;
	}
	else
	{
		return -1;
	}
}



/**
 * @brief 发送SSL封装好的数据，如果使用socket编程的，可以直接参考，如果使用AT指令编程的，那么需要自己来实现
 * @param Socketfd [in] socket id，如果是AT指令，单路链接，传入0，不用管，多路链接的，传入CIPSTART时用的通道号
 * @param Buf [in] 需要发送数据的指针
 * @param TxLen [in] 需要发送的长度
 * @return  返回发送的长度， -1表示发送失败.
 */
static int32_t SSL_SocketTx(int32_t Socketfd, void *Buf, uint16_t TxLen)
{
    struct openat_timeval tm;
    openat_fd_set WriteSet;
	int32_t Result;
	DBG_INFO("%dbyte need send", TxLen);
	Result = send(Socketfd, (uint8_t *)Buf, TxLen, 0);

	if (Result < 0)
	{
		DBG_ERROR("TCP %d %d", Result, socket_errno(Socketfd));
		return -1;
	}
    FD_ZERO(&WriteSet);
    FD_SET(Socketfd, &WriteSet);
    tm.tv_sec = 75;
    tm.tv_usec = 0;
    Result = select(Socketfd + 1, NULL, &WriteSet, NULL, &tm);
    if(Result > 0)
    {
		DBG_INFO("TCP TX OK! %dbyte", TxLen);
		return Result;
    }
    else
    {
        DBG_ERROR("TCP TX ERROR");
        return -1;
    }
}

/**
 * @brief 接收SSL封装好的数据，如果使用socket编程的，可以直接参考，如果使用AT指令编程的，那么需要自己来实现
 * @param Socketfd [in] socket id，如果是AT指令，单路链接，传入0，不用管，多路链接的，传入CIPSTART时用的通道号
 * @param Buf [in] 存放接收数据的指针
 * @param TxLen [in] 需要接收的长度，可能会超出本次接收的长度，没关系
 * @return  返回接收的长度， -1表示接收失败.
 */
static int32_t SSL_SocketRx(int32_t Socketfd, void *Buf, uint16_t RxLen)
{
    struct openat_timeval tm;
    openat_fd_set ReadSet;
	int32_t Result;
    FD_ZERO(&ReadSet);
    FD_SET(Socketfd, &ReadSet);
    tm.tv_sec = 30;
    tm.tv_usec = 0;
    Result = select(Socketfd + 1, &ReadSet, NULL, NULL, &tm);
    if(Result > 0)
    {
    	Result = recv(Socketfd, Buf, RxLen, 0);
        if(Result == 0)
        {
        	DBG_ERROR("socket close!");
            return -1;
        }
        else if(Result < 0)
        {
        	DBG_ERROR("recv error %d", socket_errno(Socketfd));
            return -1;
        }
        DBG_INFO("recv %d\r\n", Result);
		return Result;
    }
    else
    {
    	return -1;
    }
}

static void demo_network_connetck(void)
{
    T_OPENAT_NETWORK_CONNECT networkparam;

    memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    memcpy(networkparam.apn, "CMNET", strlen("CMNET"));

    iot_network_connect(&networkparam);

}

static void SSL_Task(PVOID pParameter)
{
	DEMO_SSL_MESSAGE*    msg;
	uint8_t *RxData;
	uint8_t ReConnCnt, Error, Quit;
	int32_t Ret;
	int32_t Socketfd = -1;
	SSL_CTX * SSLCtrl = SSL_CreateCtrl(1); //缓存1个session，否则下面的打印主KEY会失败
	SSL * SSLLink = NULL;
	ReConnCnt = 0;
	if (!SSLCtrl)
	{
		DBG_ERROR("!");
		Quit = 1;
	}
	else
	{
		Quit = 0;
	}
#ifdef TEST_URL
	Ret = SSL_LoadKey(SSLCtrl, SSL_OBJ_X509_CACERT, SymantecClass3SecureServerCA_G4, strlen(SymantecClass3SecureServerCA_G4), NULL);
#else
	Ret = SSL_LoadKey(SSLCtrl, SSL_OBJ_X509_CACERT, RootCert, strlen(RootCert), NULL);
	//如果是双向认证的，需要加载客户端的证书和私钥
	Ret = SSL_LoadKey(SSLCtrl, SSL_OBJ_X509_CERT, ClientCert, strlen(ClientCert), NULL);
	Ret = SSL_LoadKey(SSLCtrl, SSL_OBJ_RSA_KEY, ClientRSAKey, strlen(ClientRSAKey), NULL);
#endif
	while (!Quit)
	{
		SOCKET_CLOSE(Socketfd);
		if (SSLLink)
		{
			SSL_FreeLink(SSLLink);
			SSLLink = NULL;
		}
		iot_os_sleep(5000);	//这里最好使用timer来延迟，demo简化使用


		//这里一直检查是否连接，如果连接不上去，就重启模组，设置的定时器，定时一段时间，应该是90s左右，如果90s之内没有激活APN就重启模块
		iot_os_stop_timer(hTimer);
		iot_os_start_timer(hTimer, 90*1000);//90秒内如果没有激活APN，重启模块
		ToFlag = 0;
		while(NWState != SSL_MSG_NETWORK_LINKED)
		{
			iot_os_wait_message(hSocketTask, (PVOID)&msg);

			switch(msg->type)
			{
				case SSL_MSG_NETWORK_READY:
					iot_debug_print("[SSL] network connecting....");
					demo_network_connetck();
					break;
				case SSL_MSG_NETWORK_LINKED:
					iot_debug_print("[SSL] network connected");
					break;
				case SSL_MSG_USER_MSG_TIMER:
					DBG_ERROR("[SSL]network wait too long!");
					iot_os_sleep(500);
					iot_os_restart();
					break;
				default:
					break;

			}
		}
//  这一部分是检查socket 是否连接上
		iot_os_stop_timer(hTimer);
		DBG_INFO("start connect server");
		Socketfd = Socket_ConnectServer();
		if (Socketfd > 0)
		{
			ReConnCnt = 0;
		}
		else
		{
			ReConnCnt++;
			DBG_ERROR("retry %dtimes", ReConnCnt);
			if (ReConnCnt > SSL_RECONNECT_MAX)
			{
				iot_os_restart();
				while (1)
				{
					iot_os_sleep(5000);
				}
			}
			continue; //这个是转到那里？
		}

		DBG_INFO("start ssl handshake");
		SSLLink = SSL_NewLink(SSLCtrl, Socketfd, NULL, 0, NULL, NULL);

		if (!SSLLink)
		{
			DBG_ERROR("!");
			Quit = 1;
			continue;
		}
		Ret = SSL_HandshakeStatus(SSLLink);
		Ret = SSL_VerifyCert(SSLLink);
		if (Ret)
		{
			DBG_ERROR("ssl handshake fail %d", Ret);
			Quit = 1;
			continue;
		}
		else
		{
			DBG_INFO("ssl handshake OK");

		}

		iot_os_start_timer(hTimer, 1*1000);//1秒后发送一次HTTP请求
		ToFlag = 0;
		Error = 0;
		while(!Error && !Quit)
		{

			while (ToFlag)
			{
				iot_os_wait_message(hSocketTask, (PVOID)&msg);
				switch(msg->type)
				{
				case SSL_MSG_USER_MSG_TIMER:
					ToFlag = 0;
					DBG_INFO("HTTP GET");
					Ret = SSL_Write(SSLLink, TEST_DATA, strlen(TEST_DATA));
					if (Ret < 0)
					{
						DBG_ERROR("ssl send error %d", Ret);
						Error = 1;
					}
					else
					{
						Ret = 0;
						while (!Ret)
						{
							Ret = SSL_Read(SSLLink, &RxData);
						}

						if (Ret < 0)
						{
							DBG_ERROR("ssl receive error %d", Ret);
							Error = 1;
						}
						else
						{
							RxData[Ret] = 0;
							DBG_INFO("%s\r\n", RxData);
						}
					}
					Quit = 1;
					break;
				default:
					if (NWState != OPENAT_NETWORK_LINKED)
					{
						Error = 1;
					}
					break;
				}
				iot_os_free(msg);
			}
		}
	}
	iot_os_stop_timer(hTimer);
	SOCKET_CLOSE(Socketfd);
	if (SSLLink)
	{
		SSL_FreeLink(SSLLink);
		SSLLink = NULL;
	}
	SSL_FreeCtrl(SSLCtrl);
	while (1)
	{
		iot_os_sleep(43200 * 1000);
	}
}


static void SSL_NetworkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    DEMO_SSL_MESSAGE* msgptr = iot_os_malloc(sizeof(DEMO_SSL_MESSAGE));
    iot_debug_print("[socket] network ind state %d", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
        msgptr->type = SSL_MSG_NETWORK_LINKED;
        iot_os_send_message(hSocketTask, (PVOID)msgptr);

        NWState = SSL_MSG_NETWORK_LINKED;
        return;
    }
    else if(state == OPENAT_NETWORK_READY)
    {
        msgptr->type = SSL_MSG_NETWORK_READY;
        iot_os_send_message(hSocketTask,(PVOID)msgptr);
        NWState = SSL_MSG_NETWORK_READY;
        return;
    }
    iot_os_free(msgptr);
}

static void SSL_TimerHandle(void *pParameter)
{
	DEMO_SSL_MESSAGE *Msg = iot_os_malloc(sizeof(DEMO_SSL_MESSAGE));
	ToFlag = 1;
	Msg->type = SSL_MSG_USER_MSG_TIMER;
	iot_os_send_message(hSocketTask, (PVOID)Msg);
	iot_os_stop_timer(hTimer);

}

int appimg_enter(void *param)
{    
	iot_debug_print("[ssl] appimg_enter");
    iot_network_set_cb(SSL_NetworkIndCallBack);
	hSocketTask = iot_os_create_task(SSL_Task,
                        NULL,
                        4096,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "demo_socket_SSL");
	NWState = OPENAT_NETWORK_DISCONNECT;
	hTimer = iot_os_create_timer(SSL_TimerHandle, NULL);
	SSL_RegSocketCallback(SSL_SocketTx, SSL_SocketRx);
	iot_pmd_exit_deepsleep();

    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[ssl] appimg_exit");
}

