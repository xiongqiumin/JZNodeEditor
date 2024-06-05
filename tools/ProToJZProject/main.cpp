#include <QtCore/QCoreApplication>
#include <QDebug>
#include "ProConvert.h"

QString code = R"(
    int add(int a,int b)
    {
        return a + b;
    }
)";


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
        
    ProConvert convert;
    convert.convert(code);

    return a.exec();
}
