Luat_CSDK_Air724U 介绍
============

## 一、简介

>Luat_CSDK_Air724U是针对使用合宙Air724U(展锐8910芯片)模块而准备的一套C语言的软件开发环境，可以让客户像开发单片机一下，使用合宙的无线通信模块



## 二、软件架构

![软件架构](https://images.gitee.com/uploads/images/2020/0707/090924_c101da41_1221708.png "luat_iot_sdk_arch.png")

## 三、版本差异

&emsp;&emsp;

| core版本          | volte  | bt     | tts    | app可用空间 | 文件系统可用空间 |
| ----------------- | ------ | ------ | ------ | ----------- | ---------------- |
| csdk              | 不支持 | 不支持 | 不支持 | 1536K       | 2432K            |
| csdk_volte        | 支持   | 不支持 | 不支持 | 576K        | 2432K            |
| csdk_bt_tts       | 不支持 | 支持   | 支持   | 1216K       | 1472K            |
| csdk_bt_tts_volte | 支持   | 支持   | 支持   | 900K        | 1152K            |

注意：如果用户APP编译后实际大小接近分配的`APP可用空间`，这将不能使用远程升级全量包升级。需要将core和底层分开升级，或者放在一起使用差分升级。本版本固件正在开发阶段，可能会有不稳定的现象。远程升级前建议先进行充分的测试。以免照成不可挽回的损失。



## 四、最小系统

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

## 五、固件下载工具

> [LuatTools](http://www.openluat.com/Product/file/luatoolsV2-redirect.html)

## 六、如何编译

   >在命令行模式下，进入代码路径。 例如需要编译demo_socket项目，就进入project下，运行demo_socket.bat。 编译成功后将会在目录`hex\Air720U_CSDK_demo_socket\`生成两个个pac文件：`Air720U_CSDK_demo_socket.pac`和`Air720U_CSDK_demo_socket_APP.pac`。其中`Air720U_CSDK_demo_socket.pac`包含底层和csdk层固件，`Air720U_CSDK_demo_socket_APP.pac`仅包含csdk层固件。

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

## 八、官方应用
> [lua解析器](app/elua/LUA解析器开源文档.md)