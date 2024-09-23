#ifndef JZ_SCRIPT_FILE_H_
#define JZ_SCRIPT_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZProjectStream.h"
#include "JZClassItem.h"
#include "JZParamItem.h"
#include "JZScriptItem.h"

class JZCORE_EXPORT JZScriptFile : public JZProjectItem
{
public:
    JZScriptFile();
    virtual ~JZScriptFile();

    bool save(QString filepath);
    bool load(QString filepath);

    JZParamItem *addParamDefine(QString name);
    void removeParamDefine(QString name);
    JZParamItem *paramDefine(QString name);

    QStringList functionList() const;
    JZScriptItem *addFunction(const JZFunctionDefine &func);
    void removeFunction(QString name);
    JZScriptItem *getFunction(QString name);

    JZScriptClassItem *addClass(QString name, QString super = QString());
    void removeClass(QString name);
    JZScriptClassItem *getClass(QString className);

protected:
    QByteArray getClassData(JZScriptClassItem *item);
    QByteArray getItemData(JZProjectItem *item);
    void setItemData(JZProjectItem *item, const QByteArray &data);

    void loadScript(QDataStream &s, JZProjectItem *item);
    void saveScript(QDataStream &s, JZProjectItem *item);
};
QSharedPointer<JZScriptFile> createTempFile(JZProject *project = nullptr);

#endif