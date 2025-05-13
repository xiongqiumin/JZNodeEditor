#include <QRegularExpression>
#include <cmath>
#include <QDebug>
#include "JZFormat.h"
#include "JZNodeType.h"

bool JZFormat::init(const QString& text, QString& error)
{
    m_blocks.clear();

    enum State {
        Normal,
        InEscape,
        InPlaceholder
    };

    State state = Normal;
    QString currentLiteral;
    QString currentPlaceholder;
    int braceDepth = 0;
    int placeholderIndex = 0;

    for (int i = 0; i < text.size(); ++i) {
        QChar c = text[i];

        switch (state) {
        case Normal:
            if (c == '\\') {
                state = InEscape;
            }
            else if (c == '{') {
                if (i + 1 < text.size() && text[i + 1] == '{') {
                    // 转义的左大括号
                    currentLiteral.append('{');
                    i++; // 跳过第二个 '{'
                }
                else {
                    // 开始占位符
                    if (!currentLiteral.isEmpty()) {
                        Block block;
                        block.type = Literal;
                        block.content = currentLiteral;
                        m_blocks.append(block);
                        currentLiteral.clear();
                    }
                    state = InPlaceholder;
                    braceDepth = 1;
                    currentPlaceholder.clear();
                }
            }
            else if (c == '}') {
                error = QString("Mismatched braces '}' at position %1").arg(i);
                return false;
            }
            else {
                currentLiteral.append(c);
            }
            break;

        case InEscape:
            if (c == '{' || c == '}') {
                currentLiteral.append(c);
            }
            else {
                // 无效的转义序列
                error = QString("Invalid escape sequence '\\%1' at position %2").arg(c).arg(i - 1);
                return false;
            }
            state = Normal;
            break;

        case InPlaceholder:
            if (c == '}') {
                if (i + 1 < text.size() && text[i + 1] == '}') {
                    // 转义的右大括号
                    currentPlaceholder.append('}');
                    i++; // 跳过第二个 '}'
                }
                else {
                    // 结束占位符
                    braceDepth--;
                    if (braceDepth == 0) {
                        Block block;
                        block.type = Placeholder;
                        block.content = currentPlaceholder;
                        block.argIndex = placeholderIndex++;

                        // 解析格式规范
                        FormatSpec format;
                        if (!parseFormatSpec(currentPlaceholder, format, error)) {
                            return false;
                        }
                        block.format = format;

                        m_blocks.append(block);
                        state = Normal;
                    }
                    else {
                        currentPlaceholder.append(c);
                    }
                }
            }
            else {
                currentPlaceholder.append(c);
            }
            break;
        }
    }

    // 检查未完成的状态
    if (state == InEscape) {
        error = "Unfinished escape sequence at end of string";
        return false;
    }
    else if (state == InPlaceholder) {
        error = "Unclosed placeholder at end of string";
        return false;
    }

    // 添加最后一个文本块
    if (!currentLiteral.isEmpty()) {
        Block block;
        block.type = Literal;
        block.content = currentLiteral;
        m_blocks.append(block);
    }

    return true;
}

bool JZFormat::parseFormatSpec(const QString& spec, FormatSpec& format, QString& error)
{
    // 格式规范语法: [[fill]align][sign][#][0][width][grouping][.precision][type]

    QString s = spec.trimmed();
    int pos = 0;

    // 解析填充和对齐
    if (pos < s.size() - 1) {
        QChar fill = s[pos];
        QChar align = s[pos + 1];

        if ((align == '<' || align == '>' || align == '^' || align == '=')) {
            format.fill = fill;
            format.align = align;
            pos += 2;
        }
    }

    // 解析符号
    if (pos < s.size()) {
        QChar c = s[pos];
        if (c == '+' || c == '-' || c == ' ') {
            format.sign = c;
            pos++;
        }
    }

    // 解析替代形式 (# 标志)
    if (pos < s.size() && s[pos] == '#') {
        format.alt = true;
        pos++;
    }

    // 解析零填充 (修正逻辑)
    if (pos < s.size() && s[pos] == '0') {
        // 检查是否是零填充标志还是宽度的一部分
        if (pos + 1 < s.size() && s[pos + 1].isDigit()) {
            // 是宽度的一部分，不处理为零填充
        }
        else if (format.align == '\0') {
            // 是零填充标志
            format.align = '=';
            format.fill = '0';
            pos++;
        }
    }

    // 解析宽度 (修正逻辑)
    QRegularExpression widthRegex("^(\\d+)");
    auto widthMatch = widthRegex.match(s.mid(pos));
    if (widthMatch.hasMatch()) {
        format.width = widthMatch.captured(1).toInt();
        pos += widthMatch.capturedLength(1);
    }

    // 解析千位分隔符
    if (pos < s.size()) {
        QChar c = s[pos];
        if (c == ',' || c == '_') {
            format.grouping = c;
            pos++;
        }
    }

    // 解析精度
    if (pos < s.size() && s[pos] == '.') {
        pos++; // 跳过 '.'

        QRegularExpression precisionRegex("^(\\d+)");
        auto precisionMatch = precisionRegex.match(s.mid(pos));
        if (precisionMatch.hasMatch()) {
            format.precision = precisionMatch.captured(1).toInt();
            pos += precisionMatch.capturedLength(1);
        }
        else {
            error = "Missing precision value after '.'";
            return false;
        }
    }

    // 解析类型
    if (pos < s.size()) {
        format.type = s.mid(pos);

        // 验证类型说明符
        QRegularExpression validTypeRegex("^[sdoxXbBfFeEgGaAcspn%]?$");
        if (!validTypeRegex.match(format.type).hasMatch()) {
            error = QString("Invalid type specifier '%1'").arg(format.type);
            return false;
        }
    }

    // 检查是否有未解析的字符
    if (pos < s.size()) {
        error = QString("Invalid format specifier part '%1'").arg(s.mid(pos));
        return false;
    }

    return true;
}

QString JZFormat::formatString(const QVariantList& list)
{
    QString result;

    for (const Block& block : m_blocks) {
        if (block.type == Literal) {
            result.append(block.content);
        }
        else if (block.type == Placeholder) {
            if (block.argIndex < list.size()) {
                QVariant value = list.at(block.argIndex);
                result.append(applyFormat(value, block.format));
            }
            else {
                // 参数不足，保留占位符原样
                result.append("{").append(block.content).append("}");
            }
        }
    }

    return result;
}

int JZFormat::paramCount()
{
    int count = 0;
    for (int i = 0; i < m_blocks.size(); i++)
    {
        if (m_blocks[i].type == JZFormat::Placeholder)
            count++;
    }
    return count;
}

QString JZFormat::applyFormat(const QVariant& value, const FormatSpec& format)
{
    if (value.isNull()) {
        return "null";
    }

    int v_type = JZNodeType::variantType(value);

    // 根据类型应用不同的格式化
    if (value.type() == QVariant::Int || value.type() == QVariant::UInt ||
        value.type() == QVariant::LongLong || value.type() == QVariant::ULongLong) {
        return formatInt(value.toLongLong(), format);
    }
    else if (value.type() == QVariant::Double || v_type == Type_float) {
        return formatDouble(value.toDouble(), format);
    }
    else if (value.type() == QVariant::String) {
        return formatString(value.toString(), format);
    }
    else if (value.type() == QVariant::Char) {
        return formatChar(value.toChar(), format);
    }
    else {
        // 默认情况，转换为字符串
        return formatString(value.toString(), format);
    }
}

QString JZFormat::formatInt(qint64 value, const FormatSpec& format)
{
    QString result;
    bool isNegative = value < 0;
    quint64 absValue = isNegative ? -value : value;

    // 处理不同的整数类型
    if (format.type.isEmpty() || format.type == "d") {
        result = QString::number(absValue);
    }
    else if (format.type == "x") {
        result = QString::number(absValue, 16).toLower();
    }
    else if (format.type == "X") {
        result = QString::number(absValue, 16).toUpper();
    }
    else if (format.type == "o") {
        result = QString::number(absValue, 8);
    }
    else if (format.type == "b") {
        result = QString::number(absValue, 2);
    }
    else if (format.type == "c") {
        result = QChar((int)absValue);
    }

    // 处理替代形式
    if (format.alt) {
        if ((format.type == "x" || format.type == "X") && absValue != 0) {
            result.prepend(format.type == "x" ? "0x" : "0X");
        }
        else if (format.type == "o" && !result.startsWith('0')) {
            result.prepend('0');
        }
        else if (format.type == "b" && absValue != 0) {
            result.prepend("0b");
        }
    }

    // 处理符号
    if (isNegative) {
        result.prepend('-');
    }
    else if (format.sign == '+') {
        result.prepend('+');
    }
    else if (format.sign == ' ') {
        result.prepend(' ');
    }

    // 处理千位分隔符
    if (format.grouping == ',') {
        QString grouped;
        int len = result.length();
        int pos = 0;

        // 跳过符号
        if (result[0] == '+' || result[0] == '-' || result[0] == ' ') {
            grouped.append(result[0]);
            pos++;
        }

        int count = 0;
        for (int i = len - 1; i >= pos; i--) {
            grouped.prepend(result[i]);
            count++;
            if (count % 3 == 0 && i > pos) {
                grouped.prepend(',');
            }
        }
        result = grouped;
    }
    else if (format.grouping == '_') {
        // 类似逗号分隔，但使用下划线
        // 简化实现，实际应根据数字类型确定分组方式
        QString grouped;
        int len = result.length();
        int pos = 0;

        if (result[0] == '+' || result[0] == '-' || result[0] == ' ') {
            grouped.append(result[0]);
            pos++;
        }

        int count = 0;
        for (int i = len - 1; i >= pos; i--) {
            grouped.prepend(result[i]);
            count++;
            if (count % 3 == 0 && i > pos) {
                grouped.prepend('_');
            }
        }
        result = grouped;
    }

    // 处理宽度和对齐
    if (result.size() < format.width) {
        int padding = format.width - result.size();
        QString paddingStr(padding, format.fill);

        if (format.align == '<') {
            result = result + paddingStr;
        }
        else if (format.align == '>') {
            result = paddingStr + result;
        }
        else if (format.align == '^') {
            int leftPadding = padding / 2;
            int rightPadding = padding - leftPadding;
            result = QString(leftPadding, format.fill) + result + QString(rightPadding, format.fill);
        }
        else if (format.align == '=') {
            // 特殊对齐：符号后填充
            if (!result.isEmpty() && (result[0] == '+' || result[0] == '-' || result[0] == ' ')) {
                QChar sign = result[0];
                result = sign + paddingStr + result.mid(1);
            }
            else {
                result = paddingStr + result;
            }
        }
    }

    return result;
}

QString JZFormat::formatDouble(double value, const FormatSpec& format)
{
    QString result;
    bool isNegative = value < 0;
    double absValue = isNegative ? -value : value;

    // 确定精度
    int precision = format.precision >= 0 ? format.precision : 6;

    // 处理不同的浮点类型
    if (format.type.isEmpty() || format.type == "f" || format.type == "F") {
        // 固定点表示法
        result = QString::number(absValue, 'f', precision);
    }
    else if (format.type == "e" || format.type == "E") {
        // 科学计数法
        result = QString::number(absValue, format.type == "e" ? 'e' : 'E', precision);
    }
    else if (format.type == "g" || format.type == "G") {
        // 通用格式，自动选择f或e
        result = QString::number(absValue, format.type == "g" ? 'g' : 'G', precision);
    }
    else if (format.type == "a" || format.type == "A") {
        // 十六进制浮点表示法
        // 简化实现，实际应使用更精确的转换
        result = "0x" + QString::number(absValue, 'f', precision);
    }

    // 处理符号
    if (isNegative) {
        result.prepend('-');
    }
    else if (format.sign == '+') {
        result.prepend('+');
    }
    else if (format.sign == ' ') {
        result.prepend(' ');
    }

    // 处理替代形式（强制显示小数点）
    if (format.alt && !result.contains('.')) {
        result.append('.');
    }

    // 处理宽度和对齐
    if (result.size() < format.width) {
        int padding = format.width - result.size();
        QString paddingStr(padding, format.fill);

        if (format.align == '<') {
            result = result + paddingStr;
        }
        else if (format.align == '>') {
            result = paddingStr + result;
        }
        else if (format.align == '^') {
            int leftPadding = padding / 2;
            int rightPadding = padding - leftPadding;
            result = QString(leftPadding, format.fill) + result + QString(rightPadding, format.fill);
        }
        else if (format.align == '=') {
            if (!result.isEmpty() && (result[0] == '+' || result[0] == '-' || result[0] == ' ')) {
                QChar sign = result[0];
                result = sign + paddingStr + result.mid(1);
            }
            else {
                result = paddingStr + result;
            }
        }
    }

    return result;
}

QString JZFormat::formatString(const QString& value, const FormatSpec& format)
{
    QString result = value;

    // 处理精度（截断字符串）
    if (format.precision >= 0 && result.size() > format.precision) {
        result = result.left(format.precision);
    }

    // 处理宽度和对齐
    if (result.size() < format.width) {
        int padding = format.width - result.size();
        QString paddingStr(padding, format.fill);

        if (format.align == '<') {
            result = result + paddingStr;
        }
        else if (format.align == '>') {
            result = paddingStr + result;
        }
        else if (format.align == '^') {
            int leftPadding = padding / 2;
            int rightPadding = padding - leftPadding;
            result = QString(leftPadding, format.fill) + result + QString(rightPadding, format.fill);
        }
    }

    return result;
}

QString JZFormat::formatChar(QChar value, const FormatSpec& format)
{
    QString result(1, value);

    // 处理宽度和对齐
    if (result.size() < format.width) {
        int padding = format.width - result.size();
        QString paddingStr(padding, format.fill);

        if (format.align == '<') {
            result = result + paddingStr;
        }
        else if (format.align == '>') {
            result = paddingStr + result;
        }
        else if (format.align == '^') {
            int leftPadding = padding / 2;
            int rightPadding = padding - leftPadding;
            result = QString(leftPadding, format.fill) + result + QString(rightPadding, format.fill);
        }
    }

    return result;
}

//JZFormatBinary
bool JZFormatBinary::init(const QString& text, QString& error)
{
    m_blocks.clear();

    enum State {
        Normal,
        InPlaceholder
    };

    State state = Normal;
    QString current;
    int argIndex = 0;

    for (int i = 0; i < text.size(); ++i) {
        QChar c = text[i];

        if (c == '{')
        {
            if (current.size() != 0)
            {
                error = QString("position %1 before '{' need space").arg(i);
                return false;
            }

            state = InPlaceholder;
        }
        else if (c == "}")
        {
            Block block;
            block.type = Placeholder;
            block.argIndex = argIndex++;
            m_blocks.push_back(block);
            state = Normal;
        }
        else if(c.isSpace() || i == text.size() - 1)
        {
            if (!c.isSpace())
                current.push_back(c);

            if(current.size() != 0) 
            {    
                bool ok;
                int hex = current.toInt(&ok, 16);
                if (!ok || hex > 255)
                {
                    error = QString("Invalid value '\\%1' at position %2").arg(current).arg(i - current.size());
                    return false;
                }

                if (m_blocks.size() > 0 && m_blocks.back().type == Literal)
                {
                    m_blocks.back().content.push_back((char)hex);
                }
                else
                {
                    Block block;
                    block.argIndex = -1;
                    block.type = Literal;
                    block.content.push_back((char)hex);
                    m_blocks.push_back(block);
                }

                current.clear();
            }
        }
        else
        {
            if (state != Normal)
            {
                error = QString("Invalid value '\\%1' at position %2").arg(current).arg(i);
                return false;
            }
            current.push_back(c);
        }
    }

    if (state == InPlaceholder) {
        error = "Unclosed placeholder at end of string";
        return false;
    }

    return true;
}

QByteArray JZFormatBinary::formatBinary(const QVariantList& list)
{
    QByteArray result;

    for (int i = 0; i < m_blocks.size(); i++) 
    {
        Block& block = m_blocks[i];
        if (block.type == Literal) {
            result.append(block.content);
        }
        else if (block.type == Placeholder) 
        {
            if (list[block.argIndex].type() == QMetaType::QByteArray)
            {
                result.append(list[block.argIndex].toByteArray());
            }
            else {
                Q_ASSERT(0);
            }
        }
    }

    return result;
}

int JZFormatBinary::paramCount()
{
    int count = 0;
    for (int i = 0; i < m_blocks.size(); i++)
    {
        if (m_blocks[i].type == JZFormat::Placeholder)
            count++;
    }
    return count;
}
