# Introduction
一个保守垃圾回收器，暂时未实现任何垃圾回收功能。

这原本是CMU CS213中的一个垃圾回收实验，本以为只是简单的一个lab，但是做完后发现这个lab测试完备，功能齐全，特意用来做GC的基础。

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

# TODO
 - 基于原设计的简单垃圾回收器
 - 基于空间复制的垃圾回收器
 - Serial GC
 - Parallel GC
 - Generational GC