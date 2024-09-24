#ifndef JZNODE_UI_FILE_H_
#define JZNODE_UI_FILE_H_

#include <QDomElement>
#include "JZProjectItem.h"
#include "JZNode.h"
#include "JZNodeObject.h"

//JZUiFile
class JZCORE_EXPORT JZUiFile : public JZProjectItem
{
public:
    JZUiFile();
    virtual ~JZUiFile();

    QString xml();
    void setXml(QString xml);

    bool save(QString filepath);
    bool load(QString filepath);

    const JZParamDefine *widgetVariable(QString name);
    QList<JZParamDefine> widgets();
    
protected:
    virtual void saveToStream(QDataStream &s) const override;
    virtual bool loadFromStream(QDataStream &s) override;

    void updateDefine();
    void walkChild(const QDomElement &root);    

    QString m_xml;
    QList<JZParamDefine> m_widgets;
};



#endif
