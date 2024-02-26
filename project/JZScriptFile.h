#ifndef JZ_SCRIPT_FILE_H_
#define JZ_SCRIPT_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZProjectStream.h"
#include "JZClassItem.h"
#include "JZParamItem.h"
#include "JZScriptItem.h"

class JZParamItem;
class JZScriptFile : public JZProjectItem
{
public:
    JZScriptFile();
    virtual ~JZScriptFile();

    bool loadFromStream(JZProjectStream &s);
    bool saveToStream(JZProjectStream &s);

    JZParamDefine *addParamDefine(QString name);
    void removeParamDefine(QString name);

    JZScriptItem *addFlow(QString name);
    void removeFlow(QString name);

    JZScriptItem *addFunction(QString name, const FunctionDefine &func);
    void removeFunction(QString name);
    JZScriptItem *getFunction(QString name);

    JZScriptClassItem *addClass(QString name, QString super = QString());
    void removeClass(QString name);
    JZScriptClassItem *getClass(QString className);

protected:

};


#endif