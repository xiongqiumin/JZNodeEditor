#include <QMap>
#include "JZGCodeProgram.h"

//JZGCodeInstance
JZGCodeInstance::JZGCodeInstance()
{
    type = None;
    X = Y = Z = NAN;
    I = J = K = 0;
    R = 0;
    F = 0;
}

JZGCodeInstance::~JZGCodeInstance()
{
}

//JZGCodeProgram
JZGCodeProgram::JZGCodeProgram()
{
}
JZGCodeProgram::~JZGCodeProgram()
{
}