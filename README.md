# vrisc

只运行于操作系统上的虚拟CPU架构。

## 使用

### 构建

```bash
mkdir build
cd build
cmake ../CMakeLists.txt
make
cd ..
```

### 运行

```bash
./build/vrisc/vrisc -m 1048576 -c 1 -b ./boot/boot.bin
```

## 中断

中断是提供给软件开发者以处理机器异常以及接收用户输入的机制。

触发中断的条件有以下两种：

* 机器异常
* 外部设备通信

中断产生后，机器将ip存于x0寄存器，flg存于x1寄存器，随后机器陷入内核态并关闭中断使能标志位。

使用`iret`从中断返回，返回时将从x0和x1寄存器恢复ip和flg。

## io

在机器中，使用`in`和`out`指令访问`0`~`63`共64个端口。

## 虚拟地址空间

没有开启分页时，机器使用物理地址访问内存。

开启分页时，使用虚拟地址空间访问内存。

虚拟地址为64位地址，包含整个虚拟地址空间；虚拟地址的最高位为用户标志位，最高位置位则表示此地址为用户态地址，否则表示内核态地址。

## 指令集

### 寄存器

* 通用寄存器：16个，`x0`~`x15`
* 指令指针寄存器
* 标志位寄存器
  * ^0 相等标志
  * ^1 绝对大于标志
  * ^2 绝对小于标志
  * ^3 零标志
  * ^4 符号标志
  * ^5 溢出标志
  * ^6 中断使能标志
  * ^7 分页使能标志
  * ^8 优先级标志
  * ^9 相对大于标志
  * ^10 相对小于标志
* 堆栈寄存器组
* 中断向量表寄存器
* 内核态/用户态页表指针寄存器

### 分页机制

分页机制由`标志位寄存器`的第7位开启。

分为`用户态`和`内核态`两组页表。

顶级页目录由`内核态/用户态页表指针寄存器`储存。

通过分页机制，可以使用`虚拟地址空间`来访问内存。

一个普通物理页的大小为14KB。

#### 虚拟地址

虚拟地址为64位地址，其各位段的含义如下：

位|项目
:-|:-:
63      |用户态标志
62~54   |保留
53~44   |四级页表项
43~34   |三级页表项
33~24   |二级页表项
23~14   |一级页表项
13~0    |页内偏移

#### 页表和页表项

一个页表项8字节，一个页表最多1024个页表项；

页表的起始地址只能是16K的整数倍；

页表项为一个物理地址，指向低一级的页表起始地址，一级页表项指向物理页起始地址；

页表项数少于1024个时，需要在最后一项后再填充一个64位整数0，表示页表结束。

由于页表项的最低14位一定是0，所以在最低14位中设置了一些标志位，表明它所指的页表或物理页的相关信息，如下：

位|项目|说明
:-|:-:|:-
13~3  |保留使用   |
2     |权限标志   |复位表示只有内核态可以访问，置位表示都可以。
1     |大页标志   |置位说明此高级页表指向一个物理页，而不是下一级页表。一级页表项的此位无效。
0     |有效标志   |当此页表项是一级页表项时，复位说明此页不在内存中。其他级此位无效。

#### 大页

大页是高级页表项直接指向的物理页。

当页表项的`大页标志`置位时，高级页表项储存的值应为一个物理页的起始地址。

大页的大小：16M、16G、16T；只要你的内存足够大，就可以分配，机器不会限制。

### 基本指令集

基本指令集占有`0`~`63`的指令编码码位，提供处理器管理指令以及最基本且必需的运算和寄存器、储存器管理指令。

编号|指令格式|编码|指令名|说明
:-:|:-|:-|:-|:-
0       |nop                 |00                                    |空指令         |暂停CPU直到中断产生
1       |add r1, r2, r3      |01 r1[4],r2[4] r3[4]                  |加法
2       |sub r1, r2, r3      |02 r1[4],r2[4] r3[4]                  |减法
3       |inc r1              |03 r1[4]                              |自增1
4       |dec r1              |04 r2[4]                              |自减1
5       |cmp r1, r2          |05 r1[4],r2[4]                        |比较
6       |and r1, r2, r3      |06 r1[4],r2[4] r3[4]                  |与运算
7       |or r1, r2, r3       |07 r1[4],r2[4] r3[4]                  |或运算
8       |not r1, r2          |08 r1[4],r2[4]                        |非运算
9       |xor r1, r2, r3      |09 r1[4],r2[4] r3[4]                  |异或运算
10      |jc imm              |0a 16/32/64[4],cond[4] imm[16/32/64]  |指令跳转
11      |cc imm              |0b 16/32/64[4],cond[4] imm[16/32/64]  |函数调用       |返回地址存于x0
12      |r                   |0c                                    |从函数返回      |跳回x0所指的地址
13      |ir imm              |0d mod[8]                             |从中断返回
14      |sysc                |0e                                    |陷入内核态
15      |sysr                |0f                                    |跳回用户态
16      |loop r, imm         |10 r1[8],imm[32]                      |条件循环       |r为非零时跳转到相对地址imm处，并将r自减
17      |chl r1, r2          |11 r1[4],r2[4]                        |左位移        |将r2左移r1位
18      |chr r1, r2          |12 r1[4],r2[4]                        |右位移
19      |rol r1, r2          |13 r1[4],r2[4]                        |左旋
20      |ror r1, r2          |14 r1[4],r2[4]                        |右旋
21      |ldi imm, r          |15 8/16/32/64[4],r[4] imm[8/16/32/64] |加载立即数
22      |ldm r1, r2          |16 r1[4],r2[4]                        |从内存加载      |将r1所指的内存处的数据加载到r2中
23      |stm r1, r2          |17 r1[4],r2[4]                        |保存到内存      |将r1的内容保存到r2所指的内存中
24      |ei                  |18                                    |使能中断
25      |di                  |19                                    |关闭中断
26      |ep                  |1a                                    |使能分页
27      |dp                  |1b                                    |关闭分页
28      |mv r1/m1,r2/m2      |1c mvflg[8] r1[4],r2[4]               |移动数据       |将r1移动至r2
29      |livt r              |1d r[8]                               |加载中断向量表
30      |lkpt r              |1e r[8]                               |加载内核态页表
31      |lupt r              |1f r[8]                               |加载用户态页表
32      |lsrg r1, r2         |20 r[8]                               |加载堆栈寄存器组的寄存器
33      |ssrg r1, r2         |21 r[8]                               |保存堆栈寄存器组的寄存器
34      |initext r           |22 r[8]                               |初始化扩展指令集  |r为指令集代码
35      |destext             |23                                    |销毁扩展指令集
36      |in imm/r1, r2       |24 r2[4],mvflg[4] imm[8]/r1[4]        |端口输入       |第一个参数是端口号，第二个是储存输入的寄存器
37      |out r1, imm/r2      |25 r1[4],mvflg[4] imm[8]/r2[4]        |端口输出       |第一个是储存输入的寄存器，第二个参数是端口号
38      |cut r, imm          |26 r[4],imm[4]                        |截断寄存器数据

[jc|cc]条件指令名
:-|:-
j       |c
je      |ce
jb      |cb
js      |cs
jne     |cne
jnb     |cnb
jns     |cns
jh      |ch
jl      |cl
jnh     |cnh
jnl     |cnl
jo      |co
jz      |cz

[cond]条件码|说明
:-:|:-
0       |non
1       |equal
2       |bigger
3       |smaller
4       |n-equal
5       |n-bigger
6       |n-smaller
7       |higher
8       |lower
9       |n-higher
a       |n-lower
b       |overflow
c       |zero

[mod]中断返回模式码|说明
:-:|:-
0       |panic, 复位CPU
1       |retry, 重新执行指令
2       |skip, 执行下一个指令

[mvflg]移动指令标志位|说明
:-:|:-
0       |r1,r2
1       |r,m
2       |m,r
3       |m1,m2

### 扩展指令集

vrisc通过扩展指令集来提供如`基本数学运算扩展`、`单指令多数据扩展`等额外功能。

扩展指令集需要用`initext`与`destext`指令对进行加载和卸载操作。

扩展指令集共享库的文件名格式：libvriscext`指令集id`.`指令集名`.so

#### 基本数学运算扩展(bae)

#### 高级向量扩展(ave)

#### 单指令多数据扩展(simde)

## 设备抽象

提供了设备`/dev/vriscx`，使用协议库访问设备。

## 开发事项

* 分页。
* 接下来开始构建中断和io管理器；同时构建设备抽象，给外部提供访问中断和io的函数库。
