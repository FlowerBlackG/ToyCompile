# resources 目录

本目录下的文件会被拷贝到 build 文件夹内。

本文件夹存放需要被 ToyCompile 直接使用的文件，如：

* 自动机定义文件
* C语言语法定义文件
* 测试程序文件

## 关键文件说明

### ansi-c.tcey.yacc

经过魔改的 C99 语法规范。我们将经过我们魔改的格式称为 TCEY。该格式在语法定义文件内，加入 token-key 的定义。

### ansi-c-mod.tcey.yacc

经过删减的 C99 语法定义。删除 ToyCompile 不希望支持的部分内容定义。
