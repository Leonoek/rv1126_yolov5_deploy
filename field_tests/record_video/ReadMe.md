# Usecase



## 1.rkmedia_vi_get_frame_test

> Get NV12 video format

```shell
rkmedia_vi_get_frame_test -h
```

**output:**

> rkmedia_vi_get_frame_test: option requires an argument -- 'h'
> usage example:
>         rkmedia_vi_get_frame_test [-a [iqfiles_dir]] [-I 0] [-M 0] [-w 1920] [-h 1080][-c 10] [-d rkispp_scale0] [-o out.nv12]
>         -a | --aiq: enable aiq with dirpath provided, eg:-a /oem/etc/iqfiles/, set dirpath emtpty to using path by default, without this option aiq should run in other application
>         -M | --multictx: switch of multictx in isp, set 0 to disable, set 1 to enable. Default: 0
>         -w | --width: VI width, Default:1920
>         -h | --heght: VI height, Default:1080
>         -d | --device_name: set device node(v4l2), Default:rkispp_scale0
>         -I | --camid: camera ctx id, Default 0
>         -c | --frame_cnt: record frame, Default:-1(unlimit)
>         -o | --output: output path, Default:NULL
> Notice: fmt always NV12



## 2.rkmedia_vi_venc_test

> Get h264, h265, mjpeg video format

```shell
rkmedia_vi_venc_test -h
```

output:

> rkmedia_vi_venc_test: option requires an argument -- 'h'
> usage example:
>         rkmedia_vi_venc_test [-a [iqfiles_dir]] [-w 1920] [-h 1080][-c 150] [-d rkispp_scale0] [-e 0] [-I 0] [-M 0] [-o output.h264]
>         -a | --aiq: enable aiq with dirpath provided, eg:-a /oem/etc/iqfiles/, set dirpath emtpty to using path by default, without this option aiq should run in other application
>         -M | --multictx: switch of multictx in isp, set 0 to disable, set 1 to enable. Default: 0
>         --fps fps of vi.
>         --hdr_mode [normal hdr2 hdr3].
>         --vi_buf_cnt buffer count of vi.
>         -w | --width: VI width, Default:1920
>         -h | --heght: VI height, Default:1080
>         -c | --frame_cnt: frame number of output, Default:150
>         -I | --camid: camera ctx id, Default 0
>         -d | --device_name set pcDeviceName, Default:rkispp_scale0, Option:[rkispp_scale0, rkispp_scale1, rkispp_scale2]
>         -e | --encode: encode type, Default:h264, Value:h264, h265, mjpeg
>         -o | --output_path: output path, Default:NULL