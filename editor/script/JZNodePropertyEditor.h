#ifndef JZNODE_PROPERTY_EDITOR_H_
#define JZNODE_PROPERTY_EDITOR_H_

#include <QWidget>
#include "JZNode.h"
#include "qttreepropertybrowser.h"
#include "qtvariantproperty.h"

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
    void setPinValue(int prop_id,const QVariant &value);
    void setPropEditable(int prop_id,bool editable);

signals:
    void sigNodePropNameChanged(int nodeId,int pinId,const QString &name);
    void sigNodePropChanged(int nodeId,int pinId,const QString &value);

protected slots:
    void onValueChanged(QtProperty *pin, const QVariant &value);    

protected:
    void clear();
    void addPropList(QString name,QVector<int> list);
    QtVariantProperty *createPropName(JZNodePin *pin);
    QtVariantProperty *createProp(JZNodePin *pin);

    JZNode *m_node;    

    QtTreePropertyBrowser *m_tree;
    QtVariantPropertyManager *m_propManager;
    QtVariantEditorFactory *m_propEditor;
    QMap<int,QtVariantProperty*> m_propMap;
    QMap<int,QtVariantProperty*> m_propNameMap;
    bool m_editing;
};

#endif
