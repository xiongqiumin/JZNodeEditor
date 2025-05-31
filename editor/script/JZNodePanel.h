#ifndef JZNODE_PANEL_H_
#define JZNODE_PANEL_H_

#include <QWidget>
#include <QTreeWidget>
#include "JZNodeAbstractPanel.h"


class JZNodePanel : public JZNodeAbstractPanel
{
    Q_OBJECT

public:
    JZNodePanel(QWidget *widget = nullptr);
    ~JZNodePanel();    

    virtual void init() override;
    virtual void updateDefine() override;

    void initBasic();
    void initLocalDefine();
    void intiLogicFlow();

    void initThis(QTreeWidgetItem *root);
    void initConstParam(QTreeWidgetItem *root);

    void initAll(QTreeWidgetItem *root);
    void addModule(QTreeWidgetItem *item_root, QString name);

    void updateFunction();
    void updateThis();
    void updateLocalDefine();
    void updateGlobalVariable();

protected:
    QTreeWidgetItem *m_itemFunction;
    QTreeWidgetItem *m_itemLocalDefine;
    QTreeWidgetItem *m_itemGlobalVariable;
    QTreeWidgetItem *m_itemClassDefine;
};

#endif
