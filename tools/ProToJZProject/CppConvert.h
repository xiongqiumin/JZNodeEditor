#ifndef CPP_CONVERT_H_
#define CPP_CONVERT_H_

#include <QString>

class CppConvert
{
public:
    CppConvert();
    ~CppConvert();

    void readFile(QString path);

protected:
    QChar readChar();
    QString readWord();
    void readString();
    void readFunction();
    void readIf();
    void readWhile();
    void readFor();
    void readSwitch();
    void readExpr();
};

#endif // CPP_CONVERT_H_
