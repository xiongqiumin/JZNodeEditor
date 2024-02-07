#ifndef JZNODE_VIEW_PANEL_H_
#define JZNODE_VIEW_PANEL_H_

#include <QWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include "JZNodePropertyEditor.h"

class JZNodeViewPanel : public QWidget
{
    Q_OBJECT

public:
    JZNodeViewPanel(QWidget *widget = nullptr);
    ~JZNodeViewPanel();
    
protected slots:
    void onSearch();
    void onTreeItemClicked(QTreeWidgetItem *current, int col);    
    void onContextMenu(const QPoint &pos);

protected:
    QLineEdit *m_lineSearch;
    QTreeWidget *m_tree;
};

#endif
