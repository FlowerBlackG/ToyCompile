# ToyCompile 项目源代码

本文件夹存放的是 ToyCompile 项目源代码。

## 文件说明

### CMakeLists.txt

本项目使用 cmake 作为构建系统。本路径下的 CMakeList.txt 是顶层构建文件，里面设置了本项目的基本结构信息。在子文件夹内，还有它们自己的 CMakeList.txt，管理它们自己的构建逻辑。

### lib

导入的外部库。我们使用它们的功能。显然，它们不是我们写的，所以我们把它们单独放在一个文件夹里。

### core

核心部分。不需要考虑用户交互等繁杂的东西，只需要实现基本逻辑（如：将字符串格式的代码转换成 Token 列表）。

详细设计请看 doc 文件夹内的几个项目报告 pdf。

### main

接收用户的输入，调用 core 提供的功能，为使用者提供服务。

### utils

一些简单的小工具。这些小工具可以移植到其他项目里。
