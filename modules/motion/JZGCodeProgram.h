#ifndef JZGCODE_PROGRAM_H
#define JZGCODE_PROGRAM_H

#include <QString>
#include <QList>

class JZGCodeInstance
{
public:
    enum Type
    {
        None,
        G0,
        G1,
        G2,
        G3,
        M4,
        M5,
    };

    explicit JZGCodeInstance();
    ~JZGCodeInstance();

    Type type;
    double X;
    double Y;
    double Z;
    double I, J, K; // 参数（圆心偏移量）
    double R;
    double F;  //速度
    double S;
};

class JZGCodeProgram
{

public:
    explicit JZGCodeProgram();
    ~JZGCodeProgram();

    QList<JZGCodeInstance> instList;
};

#endif // JZGCODEPARSER_H