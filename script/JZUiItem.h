#ifndef JZNODE_UI_FILE_H_
#define JZNODE_UI_FILE_H_

#include <QDomElement>
#include "JZProjectItem.h"
#include "JZNode.h"
#include "JZNodeObject.h"
#include "JZUiBaseItem.h"

//JZUiItem
class JZUiItem : public JZUiBaseItem
{
public:
    enum UiType{
        Ui_Widget,
        Ui_Dialog,
        Ui_MainWindow,
    };

    JZUiItem();
    virtual ~JZUiItem();

    virtual JZNodeObjectWidgetDefine define();
    
    void initXml(UiType type);    
    QString xml();
    void setXml(QString xml);

    bool save(QString filepath);
    bool load(QString filepath);    
    
protected:
    virtual void saveToStream(QDataStream &s) const override;
    virtual bool loadFromStream(QDataStream &s) override;

    void updateDefine();
    void walkChild(const QDomElement &root);    

    QString m_xml;
};



#endif
