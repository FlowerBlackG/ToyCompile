# ToyCompile 快乐编译

## 简介

本项目是我们在同济大学计算机系课程《编译原理》中的大作业项目。从词法分析开始，制作类C语言识别程序。

本项目的诞生旨在为未来的同学提供更易于参考的代码，便于他们更快乐地学习编译原理课程知识，更优雅地完成课程设计项目。

## 项目阅读方法

### 准备工作

本项目通过 CMake 管理，依赖 Boost 库。对于 Windows MinGW 平台，我们已经将库内置到项目里。对于 Linux 和 MacOS 用户，请自行安装 Boost 库。

开发时，我们选择的环境是 Ubuntu Server 22.04 或 Windows + MinGW。推荐你选择与我们一样的环境，以免遇到令人不悦的问题。

对于 Windows 用户，请安装 MinGW 开发套件。下面提供一个可能能用的下载源：

> https://github.com/skeeto/w64devkit

你需要保证 make 和 g++ 程序在你的系统环境变量 path 内。当你打开命令行，输入命令 `gcc -v`，得到一些版本信息，则表明准备工作到位。

同时，你需要安装 CMake。它比 MinGW 好装多了。快去试试看。

下载项目，解压，使用令你觉得快乐的编辑器（比如 VS Code）打开该文件夹。

### 项目结构

下面说明文件夹内存储的是什么：

|   文件夹   |                             含义                             |
| :--------: | :----------------------------------------------------------: |
|  .vscode   |              与 vscode 相关设置。你可以不管它。              |
|   build    |                    代码构建时的工作路径。                    |
| references |                        部分参考材料。                        |
|   report   |    实验报告。里面包含项目结构的详细说明。有耐心可以阅读。    |
| resources  | 资源。如，识别词法的自动机、测试文件。该文件夹会被 CMake 拷贝到 build 内。 |
|    src     |                   ToyCompile 项目源代码。                    |
|   tools    | 辅助工具。例如，将 JFlap 格式自动机转换为我们自己定义的格式的工具。 |

src 路径内，内部核心部分（词法分析、语法分析等）都在 core 内。与用户交互的部分，含数据传输服务器，都在 main 内。

### 构建

使用命令行，切换到 build 路径内（如果该路径不存在，请手动创建）。

对于 Windows MinGW 用户，我们准备了两个脚本，都是以 `.ps1` 为后缀的。确保你的命令行是 powershell，不然无法运行这两个脚本。如果你是用的是 cmd，可以通过命令 `powershell` 或 `pwsh` 切换到 powershell 环境。

运行脚本 `cmake-mingw-prepare.ps1`，即可完成项目准备工作。之后，运行 `cmake-build.ps1` 即可完成项目构建。如果看到 build 路径下多出一个文件 ToyCompile.exe，则表明构建完毕。

Linux 和 MacOS 用户请按照各自的方式完成。

### 启动

ToyCompile 程序包含多个子程序，如 LexerCli, ParserCli 和 UniServer。你需要在运行时指定进入哪个子程序。

例如，你想使用命令行进行词法分析，可以执行以下指令：

```sh
./ToyCompile sLexerCli -fname:./resources/test/easy.c.txt
```

其中，`sXXX` 表示指定使用的子程序是 LexerCli。

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
