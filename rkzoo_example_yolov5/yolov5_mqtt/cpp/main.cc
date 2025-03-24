// Copyright (c) 2023 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*-------------------------------------------
                Includes
-------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yolov5.h"
#include "image_utils.h"
#include "file_utils.h"
#include "image_drawing.h"

#include "mymqtt.h"

#if defined(RV1106_1103) 
    #include "dma_alloc.hpp"
#endif

/*-------------------------------------------
                  Main Function
-------------------------------------------*/
int main(int argc, char **argv)
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_willOptions will_opts = MQTTClient_willOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    int rc;

    if (argc != 3)
    {
        printf("%s <model_path> <image_path>\n", argv[0]);
        return -1;
    }

    const char *model_path = argv[1];
    const char *image_path = argv[2];

    int ret;
    rknn_app_context_t rknn_app_ctx;
    memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));

    init_post_process();

    ret = init_yolov5_model(model_path, &rknn_app_ctx);
    if (ret != 0)
    {
        printf("init_yolov5_model fail! ret=%d model_path=%s\n", ret, model_path);
        goto out;
    }

    image_buffer_t src_image;
    memset(&src_image, 0, sizeof(image_buffer_t));
    ret = read_image(image_path, &src_image);
    printf("read done!\n");

#if defined(RV1106_1103) 
    //RV1106 rga requires that input and output bufs are memory allocated by dma
    ret = dma_buf_alloc(RV1106_CMA_HEAP_PATH, src_image.size, &rknn_app_ctx.img_dma_buf.dma_buf_fd, 
                       (void **) & (rknn_app_ctx.img_dma_buf.dma_buf_virt_addr));
    memcpy(rknn_app_ctx.img_dma_buf.dma_buf_virt_addr, src_image.virt_addr, src_image.size);
    dma_sync_cpu_to_device(rknn_app_ctx.img_dma_buf.dma_buf_fd);
    free(src_image.virt_addr);
    src_image.virt_addr = (unsigned char *)rknn_app_ctx.img_dma_buf.dma_buf_virt_addr;
    src_image.fd = rknn_app_ctx.img_dma_buf.dma_buf_fd;
    rknn_app_ctx.img_dma_buf.size = src_image.size;
#endif
    printf("now!\n");
    if (ret != 0)
    {
        printf("read image fail! ret=%d image_path=%s\n", ret, image_path);
        goto out;
    }

    object_detect_result_list od_results;

    ret = inference_yolov5_model(&rknn_app_ctx, &src_image, &od_results);
    if (ret != 0)
    {
        printf("init_yolov5_model fail! ret=%d\n", ret);
        goto out;
    }

    // 画框和概率
    char text[50];
    for (int i = 0; i < od_results.count; i++)
    {
        object_detect_result *det_result = &(od_results.results[i]);
        printf("%s @ (%d %d %d %d) %.3f\n", coco_cls_to_name(det_result->cls_id),
               det_result->box.left, det_result->box.top,
               det_result->box.right, det_result->box.bottom,
               det_result->prop);
        int x1 = det_result->box.left;
        int y1 = det_result->box.top;
        int x2 = det_result->box.right;
        int y2 = det_result->box.bottom;

        draw_rectangle(&src_image, x1, y1, x2 - x1, y2 - y1, COLOR_BLUE, 3);

        sprintf(text, "%s %.1f%%", coco_cls_to_name(det_result->cls_id), det_result->prop * 100);
        draw_text(&src_image, text, x1, y1 - 20, COLOR_RED, 10);
    }

    sprintf(text, "%s:%d", "cows num", od_results.count);
    draw_text(&src_image, text, 100, 100, COLOR_RED, 10);

    write_image("out.png", &src_image);


    

    /* 创建mqtt客户端对象 */
    if (MQTTCLIENT_SUCCESS !=
            (rc = MQTTClient_create(&client, BROKER_ADDRESS, CLIENTID,
            MQTTCLIENT_PERSISTENCE_NONE, NULL))) {
        printf("Failed to create client, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto exit;
    }

    /* 设置回调 */
    if (MQTTCLIENT_SUCCESS !=
            (rc = MQTTClient_setCallbacks(client, NULL, connlost,
            msgarrvd, NULL))) {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }

    /* 连接MQTT服务器 */
    will_opts.topicName = WILL_TOPIC;   			//遗嘱主题
    will_opts.message = "Unexpected disconnection";	//遗嘱消息
    will_opts.retained = 1; 							//保留消息
    will_opts.qos = 0;      						//QoS0

    conn_opts.will = &will_opts;
    conn_opts.keepAliveInterval = 30;   				//心跳包间隔时间
    conn_opts.cleansession = 0;         				//cleanSession标志
    conn_opts.username = USERNAME;      		//用户名
    conn_opts.password = PASSWORD;      			//密码
    if (MQTTCLIENT_SUCCESS !=
            (rc = MQTTClient_connect(client, &conn_opts))) {
        printf("Failed to connect, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }

    printf("MQTT服务器连接成功!\n");

    /* 发布上线消息 */
    pubmsg.payload = (void*)"Online";  	//消息的内容
    pubmsg.payloadlen = 6;      	//内容的长度
    pubmsg.qos = 0;             	//QoS等级
    pubmsg.retained = 1;        	//保留消息
    if (MQTTCLIENT_SUCCESS !=
        (rc = MQTTClient_publishMessage(client, WILL_TOPIC, &pubmsg, NULL))) {
        printf("Failed to publish message, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto disconnect_exit;
    }

    /* 订阅主题 dt_mqtt/led */
    if (MQTTCLIENT_SUCCESS !=
            (rc = MQTTClient_subscribe(client, LED_TOPIC, 0))) {
        printf("Failed to subscribe, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto disconnect_exit;
    }

    // /* 向服务端发布芯片温度信息 */
    /* 向服务端发布牛只信息 */
    for ( ; ; ) {

        MQTTClient_message tempmsg = MQTTClient_message_initializer;
        char temp_str[10] = {0};
        int fd;

        // /* 读取温度值 */
        // fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
        // read(fd, temp_str, sizeof(temp_str));//读取temp属性文件即可获取温度
        // close(fd);

        /* 发布温度信息 */
        // tempmsg.payload = temp_str; 			//消息的内容
        tempmsg.payload = text; 			//消息的内容
        tempmsg.payloadlen = strlen(text); 	//内容的长度
        tempmsg.qos = 0;                		//QoS等级
        tempmsg.retained = 1;       			//保留消息
        if (MQTTCLIENT_SUCCESS !=
            (rc = MQTTClient_publishMessage(client, TEMP_TOPIC, &tempmsg, NULL))) {
            printf("Failed to publish message, return code %d\n", rc);
            rc = EXIT_FAILURE;
            goto unsubscribe_exit;
        }

        sleep(10);      //每隔30秒 更新一次数据
    }

unsubscribe_exit:
    if (MQTTCLIENT_SUCCESS !=
        (rc = MQTTClient_unsubscribe(client, LED_TOPIC))) {
        printf("Failed to unsubscribe, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }
disconnect_exit:
    if (MQTTCLIENT_SUCCESS !=
        (rc = MQTTClient_disconnect(client, 10000))) {
        printf("Failed to disconnect, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }
destroy_exit:
    MQTTClient_destroy(&client);
exit:
    return rc;

out:
    deinit_post_process();

    ret = release_yolov5_model(&rknn_app_ctx);
    if (ret != 0)
    {
        printf("release_yolov5_model fail! ret=%d\n", ret);
    }

    if (src_image.virt_addr != NULL)
    {
#if defined(RV1106_1103) 
        dma_buf_free(rknn_app_ctx.img_dma_buf.size, &rknn_app_ctx.img_dma_buf.dma_buf_fd, 
                rknn_app_ctx.img_dma_buf.dma_buf_virt_addr);
#else
        free(src_image.virt_addr);
#endif
    }

    return 0;
}
