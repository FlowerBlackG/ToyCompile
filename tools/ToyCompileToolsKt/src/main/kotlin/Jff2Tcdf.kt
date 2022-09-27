/*
 * Jff 2 Tcdf
 * 将 JFLAP 文件格式转换为 ToyCompile 自动机描述格式。
 *
 * 创建于2022年9月26日
 */

/*
 * Tcdf 格式：
 * ToyCompile DFA Format
 * 用于描述一个DFA，该DFA可被ToyCompile识别。
 * 该DFA允许有未定义的转移。
 *
 * 文件为纯文本，每行表示一条信息。
 *
 * 信息有2种类别：
 *   1. 定义状态
 *   2. 定义转移
 *
 * 具体表示形式：
 *   1. 定义状态
 *     def [id: Integer] ["final" | "start" | "normal"]
 *       id 为状态编号，应该保证唯一。
 *       后追加节点备注：
 *         final: 终态节点。
 *         start: 起始节点，只应有1个。
 *         normal: 一般节点。
 *     例：
 *       def 3 final
 *       def 5 normal
 *
 *   2. 定义转移
 *     trans [id1: Integer] [id2: Integer] [ch: ASCII Integer]
 *       表示使用字符 ch，可以完成从 id1 到 id2 的转移。
 *     例：
 *       7 8 x
 *       12 17 ~
 *
 * 为方便判断结尾，在最后一行之后额外添加一行，内容为：eof
 */

/*
 * jff read 状态转移描述语句规范
 *   单字符：直接表示该字符
 *   \+单字符：转义（字符仅限：n r t）
 *   x~y：ASCII 范围：[x.code, y.code]
 *   any：任意可见字符。
 *   any but a|b|c|...
 *     特别的，用 \vln 表示 | 本身。
 *   eof：-1
 *
 * 特殊转义：
 *   \vln: |
 *   \bs: 空格
 *
 *
 * 不允许有多余空格等奇怪字符。
 */

import org.jsoup.Jsoup
import org.jsoup.nodes.Element
import java.io.BufferedWriter
import java.io.File
import kotlin.system.exitProcess

fun processStateNode(stateNode: Element, tcdfWriter: BufferedWriter) {
    val isFinal = stateNode.getElementsByTag("final").size > 0
    val isInitial = stateNode.getElementsByTag("initial").size > 0
    val isNormal = !(isFinal || isInitial)
    val id = stateNode.attr("id")
    val name = stateNode.attr("name")

    tcdfWriter.write("def ${id} ")

    if (isFinal) {
        tcdfWriter.write("final ")
    }

    if (isInitial) {
        tcdfWriter.write("start ")
    }

    if (isNormal) {
        tcdfWriter.write("normal")
    }

    tcdfWriter.write("\n")
}

/**
 * ascii 可见字母表范围。不包含空格。
 */
val asciiVisibleCharRange = 33 .. 126

fun String.decodeEscapeCh(): Char {
    return when (this[1]) {
        '\\' -> '\\'
        'n' -> '\n'
        't' -> '\t'
        'r' -> '\r'
        'v' -> when (this) {
            "\\vln" -> '|'
            else -> 'v'
        }
        'b' -> when (this) {
            "\\bs" -> ' '
            else -> 'b'
        }
        else -> Char(0)
    }
}

fun String.parseAsJffRead(): List<Int> {
    val read = this

    val res = ArrayList<Int>()

    if (read.isEmpty()) {
        return res
    }

    if (read.length == 1) {

        res.add(read[0].code)

    } else if (read[0] == '\\') {

        res.add(read.decodeEscapeCh().code)

    } else if (read == "eof") {

        res.add(-1)

    } else if (read == "any") {

        asciiVisibleCharRange.forEach { code -> res.add(code) }

    } else if (read.startsWith("any")) { // any but xxx

        val rmList = ArrayList<Int>()

        val segments = read.split(" ")
        if (segments.size >= 3 && segments[1] == "but") {
            segments[2].split("|").forEach { str ->
                if (str.length == 1) {
                    rmList.add(str[0].code)
                } else if (str.length > 1 && str[0] == '\\') {
                    rmList.add(str.decodeEscapeCh().code)
                }
            }
        }

        asciiVisibleCharRange.forEach { ch ->
            if (!rmList.contains(ch)) {
                res.add(ch)
            }
        }

    } else if (read.contains('~')) {

        val segments = read.split('~')
        if (segments.size == 2 && segments[0].length == 1 && segments[1].length == 1) {
            val beg = segments[0][0]
            val end = segments[1][0]

            val begInt = beg.code
            val endInt = end.code

            (begInt..endInt).forEach { code -> res.add(code) }
        }

    } else {

        println("failed to parse: $read")

    }

    return res
}

fun processTransitionNode(transitionNode: Element, tcdfWriter: BufferedWriter) {
    val idFrom = transitionNode.getElementsByTag("from")[0].text()
    val idTo = transitionNode.getElementsByTag("to")[0].text()
    val read = transitionNode.getElementsByTag("read")[0].text()

    read.parseAsJffRead().forEach { code ->
        tcdfWriter.write("trans $idFrom $idTo $code\n")
    }
}

fun main(args: Array<String>) {
    if (args.size < 2) {
        println("usage: jff2tcdf [jff file] [tcdf file]")
        exitProcess(0)
    }

    val jffFile = File(args[0])
    val tcdfFile = File(args[1])

    if (!tcdfFile.createNewFile()) {
        println("failed to create file: ${args[1]}")
        println("using existing one...")
    }

    // open output streams
    val tcdfOutStream = tcdfFile.outputStream()
    val tcdfWriter = tcdfOutStream.bufferedWriter()

    // open jff xml document
    val jffDocument = try {
        Jsoup.parse(jffFile)
    } catch (e: Exception) {
        println("failed to parse jff file: ${args[0]}")
        exitProcess(-2)
    }

    // 处理状态定义
    jffDocument.getElementsByTag("state").forEach { stateNode ->
        processStateNode(stateNode, tcdfWriter)
    }

    println("state process done.")

    // 处理转移
    jffDocument.getElementsByTag("transition").forEach { transitionNode ->
        processTransitionNode(transitionNode, tcdfWriter)
    }

    println("transition process done.")

    tcdfWriter.write("eof\n")

    println("conversion done.")

    // clean up
    tcdfWriter.close()
    tcdfOutStream.close()
}
