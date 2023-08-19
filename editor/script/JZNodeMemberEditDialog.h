#pragma once

#include <QDialog>
#include <QTableWidget>
#include "JZNodeValue.h"

namespace Ui { class JZNodeMemberEditDialog; }

class QTreeWidgetItem;
class JZNodeMemberEditDialog : public QDialog
{
    Q_OBJECT

public:
    JZNodeMemberEditDialog(QWidget *parent = Q_NULLPTR);
    ~JZNodeMemberEditDialog();
    
    void init(JZNode *node);
    QString className();
    QStringList paramList();

protected slots:    
    void on_lineName_returnPressed();
    void on_btnSelect_clicked();
    void on_btnMoveRight_clicked();
    void on_btnMoveLeft_clicked();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

private:    
    void updateTree();
    QTreeWidgetItem *createTreeItem(QString name,int type);
    
    QString m_className;
    JZNodeAbstractMember *m_node;
    Ui::JZNodeMemberEditDialog *ui;
};
