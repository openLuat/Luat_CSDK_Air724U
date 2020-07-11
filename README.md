Luat_CSDK_Air724U 介绍
============

## 一、简介

>Luat_CSDK_Air724U是针对使用合宙Air724U(展锐8910芯片)模块而准备的一套C语言的软件开发环境，可以让客户像开发单片机一下，使用合宙的无线通信模块

## 二、软件架构
![软件架构](https://images.gitee.com/uploads/images/2020/0707/090924_c101da41_1221708.png "luat_iot_sdk_arch.png")

## 三、最小系统

```
//最小系统
static VOID demo_task_main(PVOID pParameter)
{
	while(1)
	{
		iot_os_sleep(1000);
		iot_debug_print("demo_task_main");
	}
}

int appimg_enter(void *param)
{   
	iot_debug_print("appimg_enter");
	iot_os_create_task(demo_task_main, NULL, 
        2048, 1, OPENAT_OS_CREATE_DEFAULT, "task");
	return 0;
}

void appimg_exit(void)
{
    iot_debug_print("appimg_exit");
}
```

## 四、固件下载工具

> [LuatTools](http://www.openluat.com/Product/file/luatoolsV2-redirect.html)
	
## 五、如何编译
   >在命令行模式下，进入代码路径。 例如需要编译demo_socket项目，就进入project下，运行demo_socket.bat。 编译成功后将会在目录`hex\Air720U_CSDK_demo_socket\`生成两个个pac文件：`Air720U_CSDK_demo_socket.pac`和`Air720U_CSDK_demo_socket_APP.pac`。其中`Air720U_CSDK_demo_socket.pac`包含底层和csdk层固件，`Air720U_CSDK_demo_socket_APP.pac`仅包含csdk层固件。
   
  
## 六、应用可用空间
 
| 项目| 可用大小|
| :--------| :--: |
| FLASH |  1.5M字节   |
| RAM   |  1M字节 |
|文件系统| 1.375M字节|

## 七、支持例程
所有支持demo的编译都放在project目录下，运行对应bat就可以编译出对应demo应用
| 例程| 说明|
| :--------| :--: |
|demo_audio|录音、通道设置等功能演示|
|demo_capture|拍照功能演示|
|demo_datetime|系统时间设置|
|demo_fota|远程升级示例|
|demo_fs|文件系统操作示例|
|demo_ftp|FTP功能示例|
|demo_http|HTTP功能示例|
|demo_mqtt|mqtt功能示例|
|demo_socket|套接字接口示例|
|demo_ssl|ssl加密套接字示例|
|demo_tts|语音播报示例|
|demo_wifiloc|wifi定位示例|
|demo_zbar|二维码扫码示例|
|demo_lvgl|littleVGL UI框架示例|