# 参数声明
查阅在normal_quantization_analysis文件夹下的entire_qnt_error_analysis.txt和individual_qnt_error_analysis.txt，参数说明如下：<br>
eculidean_norm 欧氏距离(归一化)，越接近0越好<br>
cosine_norm 余弦相似度，越接近1越好<br>
eculidean 欧氏距离，越接近0越好<br>
cosine 余弦相似度，越接近1越好<br>


# 运行环境
python3.8     rknn1.7.5<br>
python my_normal_quantizition.py<br>

# 建议使用顺序
1.查看转换的模型到onnx的准确度<br>
例如你应当粗略的比较从yolov5训练出的.pt模型和经过转换的onnx的模型识别准确度<br>

2.查看从onnx到rknn转换的精度（指余弦值和欧氏距离）<br>
my_normal_quantizition.py<br>

3.对比原模型的分类结果
my_run_onnx_model.py<br>

# 注意
models和utils文件夹的来源：atk的yolov5的工程<br>