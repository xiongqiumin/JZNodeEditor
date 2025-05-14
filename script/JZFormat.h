#ifndef JZFORMAT_H
#define JZFORMAT_H

#include <QString>
#include <QList>
#include <QVariantList>
#include <QByteArray>
#include <QRegularExpression>

class JZFormat
{
public:
    enum BlockType {
        Literal,       // 普通文本块
        Placeholder    // 占位符块
    };

    struct FormatSpec {
        QString type;      // 类型说明符（d, f, s, x, etc.）
        int width = 0;     // 最小宽度
        int precision = -1;// 精度
        QChar fill = ' ';  // 填充字符
        QChar align = '>'; // 对齐方式（<, >, ^）
        QChar sign = '\0'; // 符号选项（+, -, 空格）
        bool alt = false;  // 替代形式
        bool zero = false; // 用0填充
    };

    struct Block {
        BlockType type;
        QString content;  // 对Literal是文本内容，对Placeholder是原始格式说明
        int argIndex;     // 占位符参数索引
        FormatSpec format;// 解析后的格式规范
    };

    bool init(const QString& text, QString& error);
    QString formatString(const QVariantList& list);
    int paramCount();

protected:
    bool parseFormatSpec(const QString& spec, FormatSpec& format, QString& error);
    QString applyFormat(const QVariant& value, const FormatSpec& format);
    QString formatInt(qint64 value, const FormatSpec& format);
    QString formatDouble(double value, const FormatSpec& format);
    QString formatString(const QString& value, const FormatSpec& format);
    QString formatChar(QChar value, const FormatSpec& format);

    QList<Block> m_blocks;
};

//binary
class JZFormatBinary
{
public:

    bool init(const QString& text, QString& error);
    QByteArray formatBinary(const QVariantList& list);
    int paramCount();

protected:
    enum BlockType {
        Literal,       // 普通文本块
        Placeholder    // 占位符块
    };

    struct Block {
        BlockType type;
        QByteArray content;
        int argIndex;     // 占位符参数索引
    };

    QList<Block> m_blocks;
};

#endif // JZFORMAT_H    