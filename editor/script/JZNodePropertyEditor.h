#ifndef JZNODE_PROPERTY_EDITOR_H_
#define JZNODE_PROPERTY_EDITOR_H_

#include <QWidget>
#include "JZNode.h"
#include "JZNodePropertyBrowser.h"

enum {
    PropEditor_varName,
    PropEditor_varType,
};

class JZNodePropertyEditor : public QWidget
{
    Q_OBJECT

public:
    JZNodePropertyEditor(QWidget *widget = nullptr);
    ~JZNodePropertyEditor();

    JZNode *node();

    void setNode(JZNode *node);    
    void updateNode();    

    void setPinName(int prop_id,const QString &name);
    void setPinValue(int prop_id,const QString &value);
    void setPropEditable(int prop_id,bool editable);

signals:    
    void sigNodePropChanged(int nodeId,int pinId,const QString &value);

protected slots:
    void onValueChanged(JZNodeProperty *pin, const QString &value);

protected:
    void clear();
    void addPropList(QString name,QVector<int> list);    
    JZNodeProperty *createProp(JZNodePin *pin);

    JZNode *m_node;    

    JZNodePropertyBrowser *m_tree;
    QMap<int, JZNodeProperty*> m_propMap;
    bool m_editing;
};

#endif
