#include "JZNodeDefine.h"
#include <QDataStream>
#include "JZNode.h"
#include "JZNodeExpression.h"
#include "JZNodeValue.h"
#include "JZNodeIR.h"
#include "JZNodeFunction.h"

static const char *magic = "12345678";

QByteArray NodeMagic()
{
    QByteArray result;
    QDataStream s(&result,QIODevice::WriteOnly);    
    s << QString(magic);
    //node
    s << sizeof(JZNodeGemo);
    s << sizeof(JZNodeConnect);
    s << sizeof(JZNode);
    s << sizeof(JZNodeFunction);
    s << sizeof(JZNodeBranch);
    s << sizeof(JZNodeIRCall);
    s << sizeof(JZNodeWhile);
    s << sizeof(JZNodeFunction);
    //node ir
    s << sizeof(JZNodeIR);
    s << sizeof(JZNodeIRNodeId);
    s << sizeof(JZNodeIRCall);
    s << sizeof(JZNodeIRJmp);
    s << sizeof(JZNodeIRExpr);  
    s << sizeof(JZNodeIRSet);

    return result;
}
