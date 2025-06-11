#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include <QScopeGuard>
#include "test_vision.h"
#include "JZProjectTemplate.h"
#include "modules/vision/JZVisionNode.h"


//VisionTest
VisionTest::VisionTest()
{
}

void VisionTest::testTemplateMatch()
{
    auto class_item = makeTestClass();

    
}

void VisionTest::testBrightnessDetector()
{
}

void VisionTest::testColorIdentify()
{
}

void VisionTest::testBarCode()
{
    JZBarCode bar;
    bar.init();

    Mat image = imread("C:/Users/xiong/Desktop/JZNodeEditorTest/data/barcode.png");
    bar.detect(image);
}

void VisionTest::testQrCode()
{
    JZQRCode qr;
    qr.init();

    Mat image = imread("C:/Users/xiong/Desktop/JZNodeEditorTest/data/barcode.png");
    qr.detect(image);
}

void test_vision(int argc, char *argv[])
{    
    VisionTest s;
    QTest::qExec(&s,argc,argv);
}
