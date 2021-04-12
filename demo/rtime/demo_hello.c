/*
 * @Author: your name
 * @Date: 2020-05-19 14:05:32
 * @LastEditTime: 2020-05-26 19:30:56
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \RDA8910_CSDK\USER\user_main.c
 */

#include "string.h"
#include "cs_types.h"
#include "osi_log.h"
#include "osi_api.h"
#include "am_openat.h"
#include "am_openat_vat.h"
#include "am_openat_common.h"
#include "iot_debug.h"
#include "iot_uart.h"
#include "iot_os.h"
#include "iot_gpio.h"
#include "iot_pmd.h"
#include "iot_adc.h"
#include "iot_vat.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "iot_vat.h"

int Rtime;
/*
测试网址：http://tcplab.openluat.com/
获取新的ip地址和端口号将下面的TCP_SERVER_IP和TCP_SERVER_PORT替换后下载本程序
等待http://tcplab.openluat.com/显示air模块发送的数据（RTIME)[]
实测结果：
AIR724UG开发板 移动卡 室内
数据循环发送周期 60s 测试时间20min
RTIME=1 时，平均电流=0.018241
RTIME=0 时，平均电流=0.039139
*/
#define TCP_SERVER_IP "180.97.81.180"
#define TCP_SERVER_PORT 56797

HANDLE TestTask_HANDLE = NULL;
uint8 NetWorkCbMessage = 0;
int socketfd = -1;
int cout_res = -1;
char str_res[2048];

static void SentTask(void *param)
{
    uint8 num = 0;
    int len = 0;
    char data[512] = {0};
    while (1)
    {
        if (socketfd >= 0)
        {
            // len = sprintf(data, "RDA8910 Sent:%d and res %d and %s", num,cout_res,str_res);
            len = sprintf(data, "RDA8910 Sent:%d and AT*RTIME?= %d ", num, Rtime);
            // len = sprintf(data, "RDA8910 Sent:%d", num);
            data[len] = '\0';
            iot_debug_print("[socket]---", data);
            if (len > 0)
            {
                // TCP 发送数据
                len = send(socketfd, data, len + 1, 0);
                if (len < 0)
                {
                    iot_debug_print("[socket] tcp send data False");
                }
                else
                {
                    iot_debug_print("[socket] tcp send data Len = %d", len);
                    num += 1;
                }
            }
        }
        // iot_pmd_enter_deepsleep();
        iot_os_sleep(60000);
    }
}

static void RecvTask(void *param)
{
    int len = 0;
    unsigned char data[512] = {0};
    while (1)
    {
        if (socketfd >= 0)
        {
            // TCP 接受数据
            len = recv(socketfd, data, sizeof(data), 0);
            if (len < 0)
            {
                iot_debug_print("[socket] tcp send data False");
            }
            else
            {
                iot_debug_print("[socket] tcp Recv data result = %s", data);
            }
        }
    }
}
static void TcpConnect()
{
    /*创建套接字 
        AF_INET (IPV4 网络协议) 
        支持SOCK_STREAM/SOCK_DGRAM，分别表示TCP、UDP连接
    */

    socketfd = socket(OPENAT_AF_INET, OPENAT_SOCK_STREAM, 0);
    while (socketfd < 0)
    {
        iot_debug_print("[socket] create tcp socket error");
        iot_os_sleep(3000);
    }
    // 建立TCP链接
    struct openat_sockaddr_in tcp_server_addr = {0};
    //AF_INET 的目的就是使用 IPv4 进行通信
    tcp_server_addr.sin_family = OPENAT_AF_INET;
    //远端端口，主机字节顺序转变成网络字节顺序
    tcp_server_addr.sin_port = htons((unsigned short)TCP_SERVER_PORT);
    //字符串远端ip转化为网络序列ip
    inet_aton(TCP_SERVER_IP, &tcp_server_addr.sin_addr);
    iot_debug_print("[socket] tcp connect to addr %s", TCP_SERVER_IP);
    int connErr = connect(socketfd, (const struct openat_sockaddr *)&tcp_server_addr, sizeof(struct openat_sockaddr));
    if (connErr < 0)
    {
        iot_debug_print("[socket] tcp connect error %d", socket_errno(socketfd));
        close(socketfd);
    }
    iot_debug_print("[socket] tcp connect success");
    iot_os_create_task(SentTask, NULL, 2048, 10, OPENAT_OS_CREATE_DEFAULT, "SentTask");
    // iot_os_create_task(RecvTask, NULL, 2048, 10, OPENAT_OS_CREATE_DEFAULT, "RecvTask");
}

static void TestTask(void *param)
{
    bool NetLink = FALSE;
    while (NetLink == FALSE)
    {
        T_OPENAT_NETWORK_CONNECT networkparam = {0};
        switch (NetWorkCbMessage)
        {
        case OPENAT_NETWORK_DISCONNECT: //网络断开 表示GPRS网络不可用澹，无法进行数据连接，有可能可以打电话
            iot_debug_print("[socket] OPENAT_NETWORK_DISCONNECT");
            iot_os_sleep(10000);
            break;
        case OPENAT_NETWORK_READY: //网络已连接 表示GPRS网络可用，可以进行链路激活
            iot_debug_print("[socket] OPENAT_NETWORK_READY");
            memcpy(networkparam.apn, "CMNET", strlen("CMNET"));
            //建立网络连接，实际为pdp激活流程
            iot_network_connect(&networkparam);
            iot_os_sleep(500);
            break;
        case OPENAT_NETWORK_LINKED: //链路已经激活 PDP已经激活，可以通过socket接口建立数据连接
            iot_debug_print("[socket] OPENAT_NETWORK_LINKED");
            NetLink = TRUE;
            break;
        }
    }
    if (NetLink == TRUE)
    {
        TcpConnect();
    }
    iot_os_delete_task(TestTask_HANDLE);
}

static void NetWorkCb(E_OPENAT_NETWORK_STATE state)
{
    NetWorkCbMessage = state;
}

static AtCmdRsp AtCmdCb_res(char *pRspStr)
{
    iot_debug_print("[vat]AtCmdCb_csq");
    // rspValue：AT返回的状态
    AtCmdRsp rspValue = AT_RSP_WAIT;
    // AT指令结果查找表
    char *rspStrTable[] = {"+CME ERROR", "*RTIME: ", "OK"};
    s16 rspType = -1;
    u8 i = 0;
    char *p = pRspStr + 2;
    strcat(str_res, pRspStr);
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            //获取AT*RTIME?返回的数值
            if (rspType == 1)
            {
                Rtime = STR_TO_INT(p[strlen(rspStrTable[rspType])]);
            }
            break;
        }
    }
    // 判断AT返回的状态
    switch (rspType)
    {
    case 0: /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

    case 1: /* +CSQ */
        rspValue = AT_RSP_WAIT;
        break;

    case 2: /* OK */
        cout_res++;
        break;

    default:
        break;
    }
    return rspValue;
}

/**/
VOID luat_ATCmdSend(VOID)
{
    AtCmdEntity atCmdInit[] = {
        {AT_CMD_DELAY "2000", 10, NULL},
        {"AT+WAKETIM=1" AT_CMD_END, 14, NULL},
        {"AT^TRACECTRL=0,0,0" AT_CMD_END, 20, NULL},
        {"AT*RTIME=1" AT_CMD_END, 12, NULL},
        /*{"AT*RTIME=0" AT_CMD_END, 12, NULL},//更改RTIME的值适应不同的网络环境*/
        {"AT*RTIME?" AT_CMD_END, 12, AtCmdCb_res},//RTIME查询命令

    };
    //批量执行AT命令 参数： AT命令参数 AT命令个数
    iot_vat_push_cmd(atCmdInit, sizeof(atCmdInit) / sizeof(atCmdInit[0]));
}

//main函数
int appimg_enter(void *param)
{
    //↓必须放在最前面iot_network_set_cb前不可以有延时
    iot_network_set_cb(NetWorkCb);
    iot_vat_send_cmd("AT^TRACECTRL=0,1,1\r\n", sizeof("AT^TRACECTRL=0,1,1\r\n"));

    iot_debug_print("[socket] ENTER");
    //开启优化 注意：关闭优化不能简单关闭函数，需要设置RTIME=0
    luat_ATCmdSend();

    //系统休眠
    iot_os_sleep(5000);
    iot_debug_print("[socket] SLEEP_OVER");
    //注册网络状态回调函数
    iot_debug_print("[socket] SET_CB_OVER");
    //创建一个任务
    TestTask_HANDLE = iot_os_create_task(TestTask, NULL, 2048, 10, OPENAT_OS_CREATE_DEFAULT, "TestTask");
    return 0;
}

//退出提示
void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}
