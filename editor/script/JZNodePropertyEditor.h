#ifndef JZNODE_PROPERTY_EDITOR_H_
#define JZNODE_PROPERTY_EDITOR_H_

#include <QWidget>
#include "JZNode.h"
#include "qttreepropertybrowser.h"
#include "qtvariantproperty.h"

class JZNodePropertyEditor : public QWidget
{
    Q_OBJECT

public:
    JZNodePropertyEditor(QWidget *widget = nullptr);
    ~JZNodePropertyEditor();

    JZNode *node();
    void setNode(JZNode *node);

signals:
    void sigPropUpdate(int nodeId);

protected slots:
    void onPropUpdate();

protected:
    JZNode *m_node;
    QtTreePropertyBrowser *m_tree;
    QtVariantPropertyManager *m_propManager;
    QtVariantEditorFactory *m_propEditor;
};

#endif
