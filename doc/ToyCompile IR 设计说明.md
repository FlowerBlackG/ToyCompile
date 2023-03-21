# ToyCompile IR 设计说明

## 位置

ToyCompile IR（tcir）处于语法树和汇编代码之间。

我们将基于C99文法生成的语法树转换成tcir，再将后者转换成汇编语言，再由汇编编译器转换为可重定向二进制文件。

## 文件后缀（推荐）

`tcir`

## TCIR 整体结构

### 存储形式

使用字符形式存储，存放于文件、字符串或字符流中。文件编码必须是纯 ascii，暂不支持中文等多字节符号（特殊说明除外）。

通过换行符号标记每条指令的结尾，即：每个指令一行，每条信息一行。

每行的前缀和后缀空白符号会被过滤。

通过空格或 tab 隔离符号，即每个符号之间都要有空格，符号之间不能有逗号等奇怪的东西。

### 功能块设定

IR 包含几大部分，每部分有不同功能。通过以下方式标记一个区域的开始和结尾：

```
@ begin of ${name}
${body}
@ end of ${name}
```

其中，`${name}` 是功能区名，要按规定设置。后续会有描述。

`${body}` 内是该功能区的内容。具体解析方式取决于功能区名。

功能块不允许重复出现，不允许嵌套出现。

### 单行注释

为方便加入说明信息，tcir 支持内嵌注释。

注释格式：

```
// ${comment}
```

当检测到 `//` 其后（包括其本身）内容将被忽略。当然，单双引号区间内（字符串内）的双斜线不遵循此规则。

### 虚拟寄存器

为便于表达式计算，设计2个32位虚拟寄存器：

* t0
* t1

其中，所有表达式计算结果（含函数返回值）使用 `t0` 存储。

转成 x86 汇编时，可以以 `eax` 和 `edx` 表示它们。

### 栈增长方向

从高往低增长。例，某变量地址为 `a`，下一变量地址可能是 `a - n`。

## 符号关联

```
@ begin of extlink
${body}
@ end of extlink
```

该区域用于导入导出符号，相当于8086汇编里的 `extern`。

### import 从外部导入

```
import ${symbol}
```

相当于 8086 汇编的 `extern`。

### export 导出

```
export ${symbol} ${"fun" : "var"}
```

相当于 8086 汇编的 `global`。

注意，这里会登记全局变量，但不会登记函数。

## 全局数据

```
@ begin of static-data
${body}
@ end of static-data
```

### 字符串

```
str ${access} ${value name} "${...}"
```

其中，转义符号保留。不显式表示尾0。value name 不含引号，需遵循变量命名规则。

access：

* val：不可变
* var：可变

例：

```
str val sayhi "hello world!\n"  // const char* sayhi = "hello world\n";
```

### 整形

```
int ${access} ${value name} ${length} ${value}
```

其中，length 可选值如下：

| 内容 |      备注      |
| :--: | :-------------: |
|  s8  |  signed int 8  |
|  u8  | unsigned int 8 |
| s16 |  signed int 16  |
| u16 | unsigned int 16 |
| s32 |  signed int 32  |
| u32 | unsigned int 32 |

value 为值，以 10 进制形式表示。允许有前缀负号。

例：

```
int var year s32 2022  // int year = 2022;
```

## 全文符号表

该表为后续代码区域提供辅助说明信息。记录定义在全局的函数和变量，包含不对外开放的。

```
@ begin of global-symtab
${body}
@ end of global-symtab
```

### 函数定义

```
fun ${visibility} ${name} ${argc} ${return-type} ${root-tab-id}
    ${...args}
```

\${name} 指定函数名。\${argc} 指定参数个数。\${...args} 表示参数表。

\${root-tab-id}: 根代码块的符号表id。

\${visibility}:

* internal
* visible

参数表由多个三元式组成，可以写在多行。

三元式结构：

```
${type} ${ "value" | "ptr" } ${name}
```

\${type} 和函数返回类型 ${return-type} 可选的值：

* u8
* s8
* u16
* s16
* u32
* s32

### 结构体定义

暂不支持

### 变量符号说明表

详细信息包括：

* 符号名
* 符号类型
* 字节宽度

描述格式：

```
var ${name} ${type} ${bytes}
```

## 块符号表

块符号表可以用来辅助栈上内存分配，和判断变量作用范围（虽然这个范围在 ir 内可能很难判断）。

```
@ begin of block-symtab

% begin
${tab}
% end

% begin
${tab}
% end
...
@ end of block-symtab
```

### 表 id

```
tab-id ${id}
```

标明本符号表的id。

### 父表 id

```
parent-tab-id ${id}
```

标明本符号表的父级表的 id。如果该表已经是祖先，则父 id 与自己的 id 相同。

### 符号定义

```
var ${id} ${name} ${type} ${bytes}
```

## 指令概述

指令区通过以下形式划出：

```
@ begin of instructions
${instructions}
@ end of instructions
```

### 值表示

对于常量，直接书写值。允许写负数。

对于变量，写其在**变量符号说明表**内的编号。

对于虚拟寄存器，写其后缀编号。

值种类：

* imm: 数字（立即数）
* val: 变量。数字表示变量 id，名字表示全局变量。
* vreg: 虚拟寄存器
* fval: 函数参数

访存方式：

* 不加任何标记：直接
* **暂不支持偏移寻址**

后续所有 `${value}` 都遵循此规则。

例：

```
mov vreg 0 imm 2  // t0 = 2
```

## 指令

### 标签与跳转

#### 标签定义

```
label ${name}
```

建议当成汇编里的标签看。

如果这个标签表示一个函数，在 ir 内不用考虑寄存器保护问题。后续转汇编时，结合全局函数表，判断标签是否为函数，再决定是否要保护寄存器。

对于函数定义标签，额外加入其块符号表id。

```
fun-label ${name} ${sym-tab id}
```

#### 直接跳转

```
jmp ${label-name}
```

#### 条件跳转

```
j${condition} ${label-name}
```

condition 为跳转条件，根据 `t0` 中的值判断。

可用的条件：

```
jge: x >= 0
jg: x > 0
je: x == 0
jl: x < 0
jle: x <= 0
jne: x != 0
```

#### 函数调用跳转

```
call ${label-name}
```

不负责处理调用栈创建与清理等事情。

需要负责完成部分寄存器保护等工作。

#### 函数调用返回

```
ret
```

不负责处理调用栈创建与清理等事情。

需要完成部分寄存器恢复等工作。

### 栈操作

#### push 压栈

```
push ${bytes} ${value}
```

bytes: 1, 2, 4

value：常数、虚拟寄存器、变量、全局常量

#### pushfc 函数调用压栈

```
pushfc ${bytes} ${value}
```

#### pop 弹出

```
pop ${bytes} ${value}
pop ${bytes}  // 空弹出
```

### 存取

#### mov

```
mov ${container} ${value}
```

注意，这与汇编中的 mov 是很不同的。汇编中很难直接在内存之间移动，但 tcir 允许这么做，毕竟这只是中间代码。

#### lea

```
lea ${container} ${value}
```

加载地址。

### 算术

#### add

```
add ${value1} ${value2}  // value1 = value1 + value2
```

#### sub

```
sub ${value1} ${value2}  // value1 = value1 - value2
```

#### neg

```
neg ${value1}  // value1 = -value1
```

#### 乘法和除法

```
mul ${value1} ${value2}
```

仅支持 32 位乘法。

#### and

```
and ${value1} ${value2}  // value1 = value1 & value2
```

#### or

```
or ${value1} ${value2}  // value1 = value1 | value2
```

#### xor

```
xor ${value1} ${value2}  // value1 = value1 ^ value2
```

#### not

```
not ${value}  // value = !value
```

### 比较

#### cmp

```
cmp ${value1} ${value2} ${true-condition}
```

`${true-condition}`：

* ge - greater or equal
* le - lesser or equal
* eq - equal
* ne - not equal
* g
* l

比较 value1 和 value2，t0 被设为是否满足 true-condition。

### 其他

#### xchg

```
xchg ${value1} ${value2}  // swap value1, value2
```

## 参考

- [1] 王爽. 汇编语言（第3版）. 清华大学出版社，2013
- [2] LLVM. LLVM Tutorial. https://llvm.org/docs/tutorial/
- [3] Evian Zhang. llvm ir tutorial. https://github.com/Evian-Zhang/llvm-ir-tutorial
- [4] 踌躇月光. 操作系统实现. bilibili
- [5] ZingLix. 8086汇编指令集整理. ZingLix Blog, 2018
- [6] 华为. MAPLE IR Specification. https://gitee.com/openarkcompiler/OpenArkCompiler/blob/master/doc/en/MapleIRDesign.md
- [7] 陈火旺等. 程序设计语言编译原理（第3版）. 国防工业出版社，2000
- [8] 同济大学计算机系. 第七章 语义分析和中间代码产生. 同济大学计算机系
