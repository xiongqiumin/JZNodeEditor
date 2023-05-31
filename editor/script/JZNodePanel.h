#ifndef JZNODE_PANEL_H_
#define JZNODE_PANEL_H_

#include <QWidget>
#include <QTreeWidget>
#include "JZNode.h"

class JZNodeTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
protected:
    QMimeData *mimeData(const QList<QTreeWidgetItem *> items) const;
};

class JZNodePanel : public QWidget
{
    Q_OBJECT

public:
    JZNodePanel(QWidget *widget = nullptr);
    ~JZNodePanel();    

    void init(int fileType);

protected slots:

protected:
    void initEvent(QTreeWidgetItem *root);
    void initVariable(QTreeWidgetItem *root);
    void initFunction(QTreeWidgetItem *root);
    void initExpression(QTreeWidgetItem *root);
    QTreeWidgetItem *createFolder(QString name);
    QTreeWidgetItem *createItem(JZNode *node);

    JZNodeTreeWidget *m_tree;
    int m_fileType;
};

#endif
