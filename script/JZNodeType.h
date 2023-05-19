#ifndef JZNODE_TYPE_H_
#define JZNODE_TYPE_H_

#include <QVariant>

enum
{
    Type_none,
    Type_any,
    Type_int,
    Type_int64,
    Type_double,
    Type_string,
    Type_point,
    Type_pointF,
    Type_rect,
    Type_rectF,
    Type_color,
    Type_image,
    Type_unknown,
};

class JZNodeType
{
public:
    static bool isNumber(int type);
    static int calcExprType(int type1,int type2);
    static bool canConvert(int type1,int type2);
    static bool canConvert(QList<int> type1,QList<int> type2);
};

#endif
