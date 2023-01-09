# vrisc

极简的、可扩展的虚拟处理器架构。

* vcore是vrisc的一个实现

## vrisc架构详情见[文档](docs/index.md)

## 使用

### 构建

* 终端使用

```bash
mkdir build
cd build
cmake ../CMakeLists.txt
make
cd ..
```

### 运行

```bash
sudo ./bin/vcore -m 1048576 -c 1 -b ./boot/boot.bin
```

vcore命令用法：

```raw
-m    指定内存大小
-c    指定vrisc核心数量
-b    指定boot程序文件
-t    使用外部时钟；不使用此选项则使用内部的默认时钟，频率约为500Hz
```

## 内部计时器

在`linux`下，计时器周期为2ms。

在`windows`下，计时器周期为5ms。

## 多处理器

支持多核，开启时默认启动core#0

## 设备抽象

提供了设备`/dev/vriscx`，使用协议库访问设备。

## 开发事项

* 给外部提供访问中断和io的函数库。
