#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include <QMainWindow>
#include "test_runtime.h"
#include "runtime/JZNodeUiLoader.h"
#include "JZScriptBuildInFunction.h"
#include "JZUiItem.h"

RuntimeTest::RuntimeTest()
{
}

void RuntimeTest::testFormatString()
{
    JZFormat formatter;
    QString error;
    QVariantList args;
    QString result;
    /*
    QVERIFY(!formatter.init("Unclosed { placeholder", error));
    QVERIFY(!formatter.init("Invalid escape \\a", error));
    QVERIFY(!formatter.init("Invalid specifier {0:xyz}", error));
    QVERIFY(!formatter.init("Incomplete specifier {0:.}", error));
    QVERIFY(!formatter.init("Mismatched braces }", error));

    // 测试基本字符串格式化
    QVERIFY(formatter.init("Hello {0}, your age is {1}!", error));
    args.append("Alice");
    args.append(30);

    result = formatter.formatString(args);
    QCOMPARE(result, "Hello Alice, your age is 30!");
    */
    // 测试带格式说明符的格式化
    QVERIFY(formatter.init("Number: {0:06d}, Hex: {0:#x}, Float: {1:+.2f}", error));
    args.clear();
    args.append(42);
    args.append(3.14159);

    result = formatter.formatString(args);
    QCOMPARE(result, "Number: 000042, Hex: 0x2a, Float: +3.14");

    // 测试对齐和填充
    QVERIFY(formatter.init("Left: [{0:<10}], Right: [{0:>10}], Center: [{0:^10}]", error));
    args.clear();
    args.append("test");

    result = formatter.formatString(args);
    QCOMPARE(result, "Left: [test      ], Right: [      test], Center: [   test   ]");
}

void RuntimeTest::testFormatBinary()
{
    JZFormatBinary formatter;
    QString error;

    QVERIFY(!formatter.init("0x01 {A} 02", error));       // 无效占位符
    QVERIFY(!formatter.init("0x01 0x {}", error));        // 不完整的十六进制
    QVERIFY(!formatter.init("0x01 GG {}", error));        // 无效的十六进制字符
    QVERIFY(!formatter.init("0x01 0x0G {}", error));      // 无效的十六进制字符
    QVERIFY(!formatter.init("0x01 {0} {}", error));       // 带索引的占位符

    // 测试基本二进制格式化
    QVERIFY(formatter.init("0xAA 55 {} BB", error));
    QVariantList args;
    args.append(QByteArray::fromHex("CC"));

    QByteArray result = formatter.formatBinary(args);
    QCOMPARE(result.toHex(), QByteArray("aa55ccbb"));

    // 测试多个占位符
    QVERIFY(formatter.init("0x01 {} 02 {}", error));
    args.clear();
    args.append(QByteArray::fromHex("FF"));
    args.append(QByteArray::fromHex("EE"));

    result = formatter.formatBinary(args);
    QCOMPARE(result.toHex(), QByteArray("01ff02ee"));
}

void RuntimeTest::testUiLoader()
{
    JZUiItem ui_item;
    ui_item.initXml(JZUiItem::Ui_MainWindow);

    QMainWindow w;

    JZNodeUiLoader loader;
    loader.create(&w, ui_item.xml());

    w.show();
    QTest::qWaitForWindowActive(&w);
}

void test_runtime(int argc, char *argv[])
{    
    RuntimeTest s; 
    QTest::qExec(&s,argc,argv);
}
