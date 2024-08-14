**Q:为什么使用v4l2，而不用camera api**

**A:相对而言，如果系统支持，使用系统api是最稳定的，但当你插入多个usb摄像头，而你需要打开指定摄像头的时候，系统的api往往不好使，它们只有0，1，2这样的id，不能区分摄像头型号**

**Q:我的设备能不能使用这个**

**A:你的设备拥有root权限，并且插入usb摄像的时候生成了/dev/videoX节点，那么即可正常使用。安卓开发板基本都能用**

#### 这个使用起来非常简单，kotlin代码
```
var v4L2: V4L2 = V4L2()
v4L2.open("/dev/video0")
```
#### 获取支持的大小
```
val supportSize = v4L2.getSupportSize()
```
#### 开始预览
```
v4L2.startCapture(1280, 960)
```

#### 然后不断的取回数据，内部取的是mjpeg，turbojpeg解码
```
val bitmap: Bitmap = v4L2.nextFrame()
```
#### 关闭释放资源
```
v4L2.close()
```
具体使用方式可以查看代码。一般的USB摄像头支持YUV和MJPEG，MJPEG由于是压缩格式，支持的帧率一般比较高，所以这里采用解码MJPEG的方式获得数据

#### 截图
![](https://github.com/caigp/v4l2_android/blob/master/ScreenCapture/img1.png?raw=true)