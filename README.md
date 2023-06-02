# ToyCompile 快乐编译

## 简介

本项目是我们在同济大学计算机系课程《编译原理》和《编译原理课程设计》中的作业项目。从词法分析开始，制作类C语言识别程序。

本项目的诞生旨在为未来的同学提供更易于参考的代码，便于他们更快乐地学习编译原理课程知识，更优雅地完成课程设计项目。

## 准备工作

本项目通过 CMake 管理，依赖 Boost 库（从2023年6月2日起，本项目不再依赖Boost，相关部分暂时被屏蔽。简单来说，你可以不管 Boost 了）。

开发时，我们选择的环境是 Ubuntu Server 22.04 或 Windows + MinGW。推荐你选择与我们一样的环境，以免遇到令人不悦的问题。

### Windows 用户

对于 Windows 用户，请安装 MinGW 开发套件。下面提供一个可能能用的下载源：

> [https://github.com/skeeto/w64devkit](https://github.com/skeeto/w64devkit)

你需要保证 make 和 g++ 程序在你的系统环境变量 path 内。当你打开命令行，输入命令 `gcc -v`，得到一些版本信息，则表明准备工作到位。

同时，你需要安装 CMake。它比 MinGW 好装多了。快去试试看。

> [https://cmake.org/download/](https://cmake.org/download/)

为得到更舒适的命令行，推荐安装更新版本的 powershell。前往微软商店，搜索 powershell，安装即可。

下载项目，解压，使用令你觉得快乐的编辑器（比如 VS Code）打开该文件夹。

### Linux 用户

请确保你的系统已经安装以下软件：

* git
* g++
* cmake

以 Ubuntu 为例，通过以下命令完成安装：

```bash
apt install g++ git cmake
```

## 项目文件夹结构

### .vscode

VSCode 配置文件路径。如果你使用的也是 VSCode，这些文件可以帮助你的 VSCode 理解代码位置。

### build

项目编译产物存放位置。如果这个文件夹不存在，请自行创建。开发过程中，你的命令行可以一直呆在这个文件夹内。

```bash
mkdir build
cd build
```

### doc

存放实验报告，以及 ToyCompile 中间代码格式设计说明书。

### references

参考材料。可以读读看。

### resources

资源文件夹。这个文件夹会被自动拷贝到 build 目录里。

该文件夹会存放一些程序运行时需要依赖的文件。例如，C++ 词法自动机定义和 C99 文法定义。

当然，这个文件夹里还存了几个简单的测试文件。

### src

ToyCompile 源代码路径。详见文件夹内的 Readme。

### tools

辅助完成 ToyCompile 设计的工具。例如，将 JFlap 自动机文件转换成便于 ToyCompile 读取的格式的工具。

## 构建

使用命令行，切换到 build 路径内（如果该路径不存在，请手动创建）。

```bash
mkdir build
cd build
```

本项目使用 cmake 完成管理。

```bash
cmake -G"Unix Makefiles" ../src  # Linux 用户请使用此命令
cmake -G"MinGW Makefiles" ../src  # Windows 用户请使用此命令
```

之后，通过此命令即可完成编译：

```bash
cmake --build .
```

如果你修改了头文件，最好先 clean，然后再 build：

```bash
make clean  # Linux 用户使用
mingw32-make clean  # Windows 用户使用
```

构建完毕，ToyCompile 二进制文件会在 build 文件夹里出现：

```bash
ls -lh

# 输出：
total 1.3M
-rw-rw-r-- 1 ... ...  15K Jun  2 08:30 CMakeCache.txt
drwxrwxr-x 6 ... ... 4.0K Jun  2 08:47 CMakeFiles
-rw-rw-r-- 1 ... ... 2.0K Jun  2 08:30 cmake_install.cmake
drwxrwxr-x 3 ... ... 4.0K Jun  2 08:47 core
drwxrwxr-x 3 ... ... 4.0K Jun  2 08:47 main
-rw-rw-r-- 1 ... ... 6.3K Jun  2 08:30 Makefile
drwxrwxr-x 4 ... ... 4.0K Jun  2 08:30 resources
-rwxrwxr-x 1 ... ... 1.3M Jun  2 08:47 ToyCompile  # <- 这个就是。Windows 下会带有 exe 后缀。
```

## 启动

ToyCompile 程序包含多个子程序，如 LexerCli, ParserCli 和 UniServer。你需要在运行时指定进入哪个子程序。

如果只是使用，推荐使用 UniCli，它包含 ToyCompile 的完整功能。

例如，你想使用 UniCli，进行词法分析，可以执行以下指令：

```sh
./ToyCompile sUniCli -fname:./resources/test/easy.c.txt
```

其中，`sXXX` 表示指定使用的子程序是 UniCli。

后续的每个参数，都遵循格式：\[减号] \[参数名] \[冒号] \[参数值]

其中，冒号和参数值可能会省略（对于开关型参数）。

后续参数遵循子程序的规定。一般来说，使用 -help 指令可以看到子程序支持的命令行参数，及说明。

如：

```sh
./ToyCompile sParserCli -help
```

得到如下输出（以2022年11月17日的版本为例）：

```
ToyCompile Parser CommandLine.

params:
  fname:[x]      : specify input file 'x'.
  help           : get help.
  rebuild-table  : reload parser table from tcey file.
                   if not set, parser would try to load cache
                   to improve performance.
  no-store-table : don't store built table to file.
  cache-table:[x]: specify cache table file.
  tcey:[x]       : set tcey file 'x'.
  dot-file:[x]   : store result to file 'x'.

must have:
  fname:[x]

examples:
  ParserCli -fname:resources/test/easy.c.txt -rebuild-table
```

由于源代码使用 UTF-8 编码，大陆大部分 Windows 系统使用 GBK 编码，我们不太方便让程序输出中文。当然，你可以尝试改变这个问题。

命令行具体操作方法，请通过 help 查询：

```bash
./ToyCompile help 
./ToyCompile sUniCli help
./ToyCompile sParserCli help
...
```

## 项目结构设计与实现原理

请见 src 路径内的 readme。

## ToyCompile 图形前端项目

[vozeo - ToyCompilerClient](https://github.com/vozeo/ToyCompilerClient)

## 参考项目

185XXXX GQT - 词法分析器

[Maoyao233 - ToyCC](https://github.com/maoyao233/toycc)

## 参考资料

- \[1] 陈火旺等. 程序设计语言编译原理（第 3 版）. 国防工业出版社, 2000
- \[2] Maoyao233. ToyCC. [https://github.com/Maoyao233/ToyCC](https://github.com/Maoyao233/ToyCC)
- \[3] GQT. 词法分析器. 2021
- \[4] LLVM. Clang C Language Family Frontend for LLVM. [https://clang.llvm.org/](https://clang.llvm.org/)
- \[5] jsoup. jsoup: Java HTML Parser. [https://jsoup.org/](https://jsoup.org/)
- \[6] Cplusplus.com. std::stringstream - sstream. [https://cplusplus.com/reference/sstream/stringstream/](https://cplusplus.com/reference/sstream/stringstream/)
- \[7] Jutta Degener. ANSI C Yacc grammar. 2004
- \[8] Jutta Degener. ANSI C grammar, Lex specification. 2017
- \[9] 枫铃树. 红黑树详解（下）（红黑树的删除操作）. CSDN, 2022
- \[10] ISO. ISO/IEC 9899:1999 Programming languages — C, 1999
