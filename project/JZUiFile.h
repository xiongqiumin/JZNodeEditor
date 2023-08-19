#ifndef JZNODE_UI_FILE_H_
#define JZNODE_UI_FILE_H_

#include "JZProjectItem.h"
#include "JZNode.h"
#include "JZNodeObject.h"

//JZUiFile
class JZUiFile : public JZProjectItem
{
public:
    JZUiFile();
    virtual ~JZUiFile();

    QString xml();
    void setXml(QString xml);

    virtual void saveToStream(QDataStream &s);
    virtual void loadFromStream(QDataStream &s);

    QList<JZParamDefine> widgets();
    void updateDefine(JZNodeObjectDefine &define);

protected:
    QString m_xml;
};



#endif
