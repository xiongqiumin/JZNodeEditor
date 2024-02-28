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

    bool save(QString filepath);
    bool save(QString filepath,QList<JZProjectItem*> items);
    bool load(QString filepath);

    JZParamItem *addParamDefine(QString name);
    void removeParamDefine(QString name);
    JZParamItem *paramDefine(QString name);

    JZScriptItem *addFlow(QString name);
    void removeFlow(QString name);
    JZScriptItem *flow(QString name);

    JZScriptItem *addFunction(QString name, const FunctionDefine &func);
    void removeFunction(QString name);
    JZScriptItem *getFunction(QString name);

    JZScriptClassItem *addClass(QString name, QString super = QString());
    void removeClass(QString name);
    JZScriptClassItem *getClass(QString className);

protected:
    QByteArray getItemData(JZProjectItem *item);
    void loadScript(QDataStream &s, JZProjectItem *item);
    void saveScript(QDataStream &s, JZProjectItem *item);

    QMap<JZProjectItem*, QByteArray> m_itemCache;
};


#endif