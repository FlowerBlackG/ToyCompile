# 辅助工具

## ToyCompileToolsKt

功能：将 JFlap 自动机定义文件，转换成易于被 ToyCompile 理解的文件。

词法识别部分，我们使用 JFlap 绘制一个自动机（位于 resources 文件夹内）。之后，用本工具将自动机从 jff 格式转换成 tcdf 格式，后者作为 ToyCompile 词法分析模块的输入。

tcdf 格式详见 `ToyCompileToolsKt/src/main/kotlin/Jff2Tcdf.kt`
