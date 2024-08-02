#ifndef JZNODE_VIEW_PANEL_H_
#define JZNODE_VIEW_PANEL_H_

#include <QWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include "JZNodePropertyEditor.h"

class JZNodeView;
class JZNodeViewPanel : public QWidget
{
    Q_OBJECT

public:
    JZNodeViewPanel(QWidget *widget = nullptr);
    ~JZNodeViewPanel();

    void setView(JZNodeView *view);
    void updateFlow(JZScriptItem *file);
    
protected slots:
    void onSearch();
    void onTreeItemClicked(QTreeWidgetItem *current, int col);    
    void onContextMenu(const QPoint &pos);

protected:
    void createNode(QTreeWidgetItem *parent, JZNode *node, JZNode *lca_node);
    void addFlow(QTreeWidgetItem *parent, QTreeWidgetItem *lca_parent,JZNode *node, QList<int> flowList);
    QString nodeText(JZNode *node);
    QString inputText(JZNode *node, int pin_id);
    JZNode *nextNode(JZNode *node,int pin_id);
    JZNode *findLCA(JZNode *node, QList<int> node_list);    

    QLineEdit *m_lineSearch;
    QMap<JZNode*, JZNode*> m_lcaMap;
    QTreeWidget *m_tree;
    JZScriptItem *m_file;
    JZNodeView *m_view;
};

#endif
