/***************************************************************
 Copyright © ALIENTEK Co., Ltd. 1998-2021. All rights reserved.
 文件名 : mqttClient.c
 作者 : 邓涛
 版本 : V1.0
 描述 : 开发板上的MQTT客户端应用程序示例代码
 其他 : 无
 论坛 : www.openedv.com
 日志 : 初版 V1.0 2021/7/20 邓涛创建
 ***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "MQTTClient.h"     //包含MQTT客户端库头文件

/* ########################宏定义##################### */
// #define BROKER_ADDRESS  "tcp://iot.ranye-iot.net:1883"  //然也物联平台社区版MQTT服务器地址
#define BROKER_ADDRESS  "mqtt://broker.emqx.io"  //然也物联平台社区版MQTT服务器地址

/* 客户端id、用户名、密码 *
 * 当您成功申请到然也物联平台的社区版MQTT服务后
 * 然也物联工作人员会给你发送8组用于连接社区版MQTT服务器
 * 的客户端连接认证信息：也就是客户端id、用户名和密码
 * 注意一共有8组，您选择其中一组覆盖下面的示例值
 * 后续我们使用MQTT.fx或MQTTool的时候 也需要使用一组连接认证信息
 * 去连接社区版MQTT服务器！
 * 由于这是属于个人隐私 笔者不可能将自己的信息写到下面 */
#define CLIENTID        "mqttx_ming" 		//客户端id
#define USERNAME        "tests"     	//用户名
#define PASSWORD        "hello"        	//密码

/* 然也物联社区版MQTT服务为每个申请成功的用户
 * 提供了个人专属主题级别，在官方发给您的微信信息中
 * 提到了
 * 以下 dt_mqtt/ 便是笔者的个人主题级别
 * dt_mqtt其实就是笔者申请社区版MQTT服务时注册的用户名
 * 大家也是一样，所以你们需要替换下面的dt_mqtt前缀
 * 换成你们的个人专属主题级别（也就是您申请时的用户名）
 */
#define WILL_TOPIC      "dt_mqtt/will"      	//遗嘱主题
#define LED_TOPIC       "dt_mqtt/led"       	//LED主题
#define TEMP_TOPIC      "dt_mqtt/temperature" 	//温度主题
/* ################################################# */

static int msgarrvd(void *context, char *topicName, int topicLen,
            MQTTClient_message *message)
{
    if (!strcmp(topicName, LED_TOPIC)) {	//校验消息的主题
        if (!strcmp("2", (const char*)message->payload)) 	//如果接收到的消息是"2"则设置LED为呼吸灯模式
            system("echo heartbeat > /sys/class/leds/user-led/trigger");
        if (!strcmp("1", (const char*)message->payload)) {	//如果是"1"则LED常量
            system("echo none > /sys/class/leds/user-led/trigger");
            system("echo 1 > /sys/class/leds/user-led/brightness");
        }
        else if (!strcmp("0", (const char*)message->payload)) {//如果是"0"则LED熄灭
            system("echo none > /sys/class/leds/user-led/trigger");
            system("echo 0 > /sys/class/leds/user-led/brightness");
        }

        // 接收到其它数据 不做处理
    }

    /* 释放占用的内存空间 */
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    /* 退出 */
    return 1;
}

static void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("    cause: %s\n", cause);
}