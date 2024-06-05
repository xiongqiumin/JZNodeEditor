#ifndef PRO_CONVERT_H_
#define PRO_CONVERT_H_

#include <QStringList>
#include "as_parser.h"

class ProConvert
{
public:
    ProConvert();
    ~ProConvert();

    void convert(QString script);

protected:
    QString printNode(asCScriptNode *node);
    void visitNode(asCScriptNode *node,int level,QString &result);

    void toFunction(asCScriptNode *node);
    void toClass(asCScriptNode *node);

    QString m_code;
    QStringList m_fileList;
};


#endif // ! PRO_CONVERT_H_
