#include "CppConvert.h"

CppConvert::CppConvert()
{

}

CppConvert::~CppConvert()
{

}

void CppConvert::readFile(QString path)
{
    while (true)
    {
        QChar c = readChar();
        if (c == '#')
        {

        }
        else
        {
            readDefine();
            readFunction();
        }        
    }    
}