#include "JZParamHelper.h"
#include "JZScriptEnvironment.h"

JZParamCoor JZParamHelper::splitMember(const QString &name)
{
    JZParamCoor coor;
    QStringList list = name.split(".");
    if(list.size() == 1)
    {
        coor.baseName = list[0];
    }
    else
    {
        coor.baseName = list[0];
        coor.memberList = list.mid(1);
    }
    return coor;
}

const JZParamDefine *JZParamHelper::memberDefine(const JZNodeObjectDefine* obj_def, const QStringList& obj_list)
{
    auto obj_inst = obj_def->manager;

    for (int i = 0; i < obj_list.size() - 1; i++)
    {
        auto ref = obj_def->param(obj_list[i]);
        obj_def = obj_inst->meta(ref->type);    
        if (!obj_def)
            return nullptr;
    }

    return obj_def->param(obj_list.back());
}