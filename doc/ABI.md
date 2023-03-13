# ABI

## System V ABI for amd64

被调用方保存：

rbx, rsp, rbp, r12, r13, r14, r15

调用者保存：

其他（rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11）

函数返回：

rax

（好吧上面这些本程序都不支持。因为懒得思考了，直接按 i386 生成了......）
