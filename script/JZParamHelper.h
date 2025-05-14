#ifndef JZ_PARAM_HELPER_H_
#define JZ_PARAM_HELPER_H_

#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"

class JZParamCoor
{
public:
    QString baseName;
    QStringList memberList;    
};


class JZParamHelper
{
public:
    static JZParamCoor splitMember(const QString &name);
    static const JZParamDefine* memberDefine(const JZNodeObjectDefine* obj_def, const QString& member);
    static const JZParamDefine* memberDefine(const JZNodeObjectDefine *obj_def, const QStringList &memberList);
};


#endif