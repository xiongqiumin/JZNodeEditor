#ifndef JZNODE_PROPERTY_EDITOR_H_
#define JZNODE_PROPERTY_EDITOR_H_

#include <QWidget>
#include <functional>
#include "JZNode.h"
#include "3rd/JZCommon/jzWidgets/JZPropertyBrowser.h"
#include "JZNodeGraphItem.h"
#include "JZNodeParamEditWidget.h"

enum {
    PropEditor_varName,
    PropEditor_varType,
};

class JZNodeView;
class JZNodePropertyEditor : public QWidget
{
    Q_OBJECT

public:
    JZNodePropertyEditor(QWidget *widget = nullptr);
    ~JZNodePropertyEditor();

    void setView(JZNodeView *view);

    JZNode *node();
    void setNode(JZNode *node);    
    void updateNode();    

    void setPinName(int prop_id,const QString &name);
    void setPinValue(int prop_id,const QString &value);
    void setPropEditable(int prop_id,bool editable);

signals:    
    void sigNodePropChanged(int nodeId,int pinId,const QString &value);

protected slots:
    void onValueChanged(JZProperty *pin, const QVariant &value);

protected:
    struct PropTrans{
        std::function<JZProperty*(QString name)> creator;
        std::function<QVariant(const QString &)> fromString;
        std::function<QString(const QVariant &)> toString;
    };

    void clear();
    void initTransMap();
    void addPropList(QString name,const QList<int> &list);    

    JZProperty *createPropValue(JZNodeGraphItem::Block *block);
    void setPropValue(JZProperty *pin,const QString &value);
    QString propValue(JZProperty *pin);

    
    JZNodeView  *m_view;

    JZNode *m_node;    
    JZNodeGraphItem *m_item;

    JZPropertyBrowser *m_tree;
    bool m_editing;
    QMap<int, JZProperty*> m_propMap;
    QMap<JZProperty*,QString> m_propTrans;
    
    QMap<QString,PropTrans> m_transMap;
};

#endif
