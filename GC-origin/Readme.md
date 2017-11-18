# Introduction
 - Finished at 2017/11/14
 - 这是一份作业的简单答案，代码排版混乱，思路不清，原本运行于Virtual Box下的Windows XP。为了移植到Win10 + VS2017做了些许修改。
 - 能够通过基本的测试
## 设计
 - 使用伸展树保存已分配的Block
 - 使用循环链表保存空闲的Block
 - 无任何压缩，长时间运行肯定会产生大量的外部碎片，因而不稳定

# Files
 - malloc.h, malloc.c: GC，内存分配相关
 - dump.h, dump.c: 提供与log类似操作
 - memlib.h, memlib.c: 提供堆相关操作
 - useful.h: 常用工具函数声明
 - driver.c: main, 测试

# Support Files
 - splay_tree.c: 伸展树
 - getopt.h, getopt.c: 从字符串中获取参数
 - ftime.h, ftime_win32.h: 提供计时函数
 - tracefiles.h: 测试文件
 - unistd.h
 - tailor.h

# Environment
 - Windows 10
 - Visual Studio 2017
 - Only compiled with 32-bit option will work
 