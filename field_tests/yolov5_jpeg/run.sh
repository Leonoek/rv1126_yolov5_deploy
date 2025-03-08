#!/bin/bash
echo "hahaha"
# 执行第一个程序
./rkmedia_venc_jpeg_test -a /etc/iqfiles -o ./model -w 640 -h 368
#./rkmedia_venc_jpeg_test -o ./model -w 640 -h 368

# 检查第一个程序的退出状态，如果成功则执行第二个程序
if [ $? -eq 0 ]; then
    ./rknn_yolov5_demo ./model/pre_yolov5s.rknn ./model/test_jpeg0.jpeg
else
    echo "rknn_yolov5_demo 运行失败，程序终止。"
fi
