#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"

#define SOCKET_MSG_NETWORK_READY (0)
#define SOCKET_MSG_NETWORK_LINKED (1)
typedef struct {
    UINT8 type;
    UINT8 data;
}DEMO_SOCKET_MESSAGE;

#define socket_dbg iot_debug_print

#define DEMO_SERVER_TCP_IP "121.40.198.143"
#define DEMO_SERVER_TCP_PORT 12415

#define DEMO_SERVER_UDP_IP "121.40.170.41"
#define DEMO_SERVER_UDP_PORT 12414                                

static HANDLE g_s_socket_task;

static int demo_socket_tcp_recv(int socketfd)
{
    unsigned char recv_buff[64] = {0};
    int recv_len;

    // TCP 接受数据
    recv_len = recv(socketfd, recv_buff, sizeof(recv_buff), 0);
    socket_dbg("[socket] tcp recv result %d data %s", recv_len, recv_buff);
   
    return recv_len;
}

static int demo_socket_tcp_send(int socketfd)
{
    int send_len;

	char data[1024] = {0};

	memset(data, 0x32, 1024);

    // TCP 发送数据
    send_len = send(socketfd, data, 1024, 0);
    socket_dbg("[socket] tcp send data result = %d", send_len);
    return send_len;
}


static int demo_socket_tcp_connect_server(void)
{
    int socketfd;
    int connErr;
    struct openat_sockaddr_in tcp_server_addr; 
    
    // 创建tcp socket
    socketfd = socket(OPENAT_AF_INET,OPENAT_SOCK_STREAM,0);
    if (socketfd < 0)
    {
        socket_dbg("[socket] create tcp socket error");
        return -1;
    }
       
    socket_dbg("[socket] create tcp socket success");
    
    // 建立TCP链接
    memset(&tcp_server_addr, 0, sizeof(tcp_server_addr)); // 初始化服务器地址  
    tcp_server_addr.sin_family = OPENAT_AF_INET;  
    tcp_server_addr.sin_port = htons((unsigned short)DEMO_SERVER_TCP_PORT);  
    inet_aton(DEMO_SERVER_TCP_IP,&tcp_server_addr.sin_addr);

    socket_dbg("[socket] tcp connect to addr %s", DEMO_SERVER_TCP_IP);
    connErr = connect(socketfd, (const struct sockaddr *)&tcp_server_addr, sizeof(struct openat_sockaddr));
    

    if (connErr < 0)
    {
        socket_dbg("[socket] tcp connect error %d", socket_errno(socketfd));
        close(socketfd);
        return -1;
    }
    socket_dbg("[socket] tcp connect success");

    return socketfd;
}

static void demo_socket_tcp_client()
{
    int socketfd, ret, err, count = 0;
    struct openat_timeval tm;
    openat_fd_set readset;

    tm.tv_sec = 2;
    tm.tv_usec = 1;

    socketfd = demo_socket_tcp_connect_server();

    if (socketfd >= 0)
    {
        while(1)
        {
            ret = demo_socket_tcp_send(socketfd);
            if(ret < 0)
            {
            	err = socket_errno(socketfd);
				socket_dbg("[socket] send last error %d", err);
            	if(err != OPENAT_EWOULDBLOCK)
            	{
                	//break;
            	}
				else
				{
					iot_os_sleep(10);
					continue;
				}
            }

            OPENAT_FD_ZERO(&readset);
            OPENAT_FD_SET(socketfd, &readset);
			iot_debug_print("socket select addr: %08x ", select);
            ret = select(socketfd+1, &readset, NULL, NULL, &tm);
            if(ret > 0)
            {
    			ret = demo_socket_tcp_recv(socketfd);
                if(ret == 0)
                {
                    socket_dbg("[socket] recv close");
                    iot_os_sleep(1000);
                }
                else if(ret < 0)
                {
                    socket_dbg("[socket] recv error %d", socket_errno(socketfd));
                    iot_os_sleep(1000);
                }
            }
            else if(ret == 0)
            {
                socket_dbg("[socket] select timeout");
            }
			if(++count >= 5)
			{
				socket_dbg("[socket] tcp loop end");
				break;
			}
			iot_os_sleep(100);
        }
        close(socketfd);
    }
}


static int demo_socket_udp_connect_server(void)
{
    int socketfd;	
    // 创建tcp socket
    socketfd = socket(OPENAT_AF_INET,OPENAT_SOCK_DGRAM,0);
    if (socketfd < 0)
    {
        socket_dbg("[socket] create udp socket error");
        return -1;
    }
    socket_dbg("[socket] create udp socket success");

    return socketfd;
}


static int demo_socket_udp_send(int socketfd)
{
    int send_len;
	struct openat_sockaddr_in udp_server_addr; 

	memset(&udp_server_addr, 0, sizeof(udp_server_addr)); // 初始化服务器地址  
    udp_server_addr.sin_family = OPENAT_AF_INET;  
    udp_server_addr.sin_port = htons((unsigned short)DEMO_SERVER_UDP_PORT);  
    inet_aton(DEMO_SERVER_UDP_IP,&udp_server_addr.sin_addr);

    // UDP 发送数据
    send_len = sendto(socketfd, "hello i'm client", strlen("hello i'm client"), 0,
    				  (struct sockaddr*)&udp_server_addr, sizeof(struct openat_sockaddr));
    socket_dbg("[socket] udp send [hello i'm client] result = %d", send_len);
    return send_len;
}

static int demo_socket_udp_recv(int socketfd)
{
    unsigned char recv_buff[64] = {0};
    int recv_len;
	openat_socklen_t udp_server_len;

	struct openat_sockaddr_in udp_server_addr; 

	memset(&udp_server_addr, 0, sizeof(udp_server_addr)); // 初始化服务器地址  
    udp_server_addr.sin_family = OPENAT_AF_INET;  
    udp_server_addr.sin_port = htons((unsigned short)DEMO_SERVER_UDP_PORT);  
    inet_aton(DEMO_SERVER_UDP_IP,&udp_server_addr.sin_addr);
	udp_server_len = sizeof(udp_server_addr);

    // UDP 接受数据
    recv_len = recvfrom(socketfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr*)&udp_server_addr, &udp_server_len);
    socket_dbg("[socket] udp recv result %d data %s", recv_len, recv_buff);
   
    return recv_len;
}


static void demo_socket_udp_client()
{
    int socketfd, ret, err, count = 0;
    

    socketfd = demo_socket_udp_connect_server();

    if (socketfd >= 0)
    {
        while(1)
        {
            ret = demo_socket_udp_send(socketfd);
            if(ret < 0)
            {
            	err = socket_errno(socketfd);
				socket_dbg("[socket] send last error %d", err);
            	if(err != OPENAT_EWOULDBLOCK)
            	{
                	break;
            	}
				else
				{
					iot_os_sleep(200);
					continue;
				}
            }
			//阻塞读取
            ret = demo_socket_udp_recv(socketfd);
            if(ret <= 0)
            {
                socket_dbg("[socket] recv error %d", socket_errno(socketfd));
                break;
            }

			if(++count >= 5)
			{
				socket_dbg("[socket] udp loop end");
				break;
			}
        }
    }
}

static void demo_gethostbyname(void)
{
    //域名解析

    char *name = "www.baidu.com";
    struct openat_hostent *hostentP = NULL;
    char *ipAddr = NULL;

    //获取域名ip信息
    hostentP = gethostbyname(name);

    if (!hostentP)
    {
        socket_dbg("[socket] gethostbyname %s fail", name);
        return;
    }

    // 将ip转换成字符串
    ipAddr = ipaddr_ntoa((const openat_ip_addr_t *)hostentP->h_addr_list[0]);
    
    socket_dbg("[socket] gethostbyname %s ip %s", name, ipAddr);
}


static void demo_network_connetck(void)
{
    T_OPENAT_NETWORK_CONNECT networkparam;
    
    memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    memcpy(networkparam.apn, "CMNET", strlen("CMNET"));

    iot_network_connect(&networkparam);

}

static void demo_networkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    DEMO_SOCKET_MESSAGE* msgptr = iot_os_malloc(sizeof(DEMO_SOCKET_MESSAGE));
    socket_dbg("[socket] network ind state %d", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
        msgptr->type = SOCKET_MSG_NETWORK_LINKED;
        iot_os_send_message(g_s_socket_task, (PVOID)msgptr);
        return;
    }
    else if(state == OPENAT_NETWORK_READY)
    {
        msgptr->type = SOCKET_MSG_NETWORK_READY;
        iot_os_send_message(g_s_socket_task,(PVOID)msgptr);
        return;
    }
    iot_os_free(msgptr);
}

static void demo_socket_task(PVOID pParameter)
{
    DEMO_SOCKET_MESSAGE*    msg;
    socket_dbg("[socket] wait network ready....");
    BOOL sock = FALSE;

    while(1)
    {
        iot_os_wait_message(g_s_socket_task, (PVOID)&msg);

        switch(msg->type)
        {
            case SOCKET_MSG_NETWORK_READY:
                socket_dbg("[socket] network connecting....");
                demo_network_connetck();
                break;
            case SOCKET_MSG_NETWORK_LINKED:
                socket_dbg("[socket] network connected");				
                if(!sock)
                {
				demo_gethostbyname(); 
				//demo_socket_udp_client();
     			demo_socket_tcp_client();
                sock = TRUE;
                }
                break;
        }

        iot_os_free(msg);
    }
}

void demo_socket_init(void)
{ 
    socket_dbg("[socket] demo_socket_init");

    //注册网络状态回调函数
    iot_network_set_cb(demo_networkIndCallBack);

    g_s_socket_task = iot_os_create_task(demo_socket_task,
                        NULL,
                        4096,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "demo_socket");
}

int appimg_enter(void *param)
{    
    socket_dbg("[socket] appimg_enter");
	demo_socket_init();

    return 0;
}

void appimg_exit(void)
{
    socket_dbg("[socket] appimg_exit");
}

