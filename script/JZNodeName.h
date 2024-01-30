#ifndef JZ_NODE_NAME_H_
#define JZ_NODE_NAME_H_

#include <QString>

class JZNodeName
{
public:
    static bool isVaildName(QString name);
    static QString memberName(QString name);

};


#endif // !JZ_NODE_NAME_H_
