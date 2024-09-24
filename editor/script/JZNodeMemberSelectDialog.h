#ifndef JZNODE_MEMBER_SELECT_DIALOG_H_
#define JZNODE_MEMBER_SELECT_DIALOG_H_

#include <QDialog>
#include "UiCommon.h"
#include "JZNode.h"
#include "JZNodeValue.h"

namespace Ui { class JZNodeMemberSelectDialog; }

//JZNodeGroupEditDialog
class JZProject;
class JZNodeMemberSelectDialog : public QDialog
{
    Q_OBJECT

public:
    JZNodeMemberSelectDialog(QWidget *p = nullptr);
    ~JZNodeMemberSelectDialog();

    void init(JZProject *project,QString className);

    QString className();
    QStringList paramList();

protected slots:
    void on_lineName_returnPressed();
    void on_btnSelect_clicked();
    void on_btnMoveRight_clicked();
    void on_btnMoveLeft_clicked();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

protected:
    void updateTree();
    QTreeWidgetItem *createTreeItem(QString name, int type);    

    QString m_className;
    JZProject *m_project;

    Ui::JZNodeMemberSelectDialog *ui;
};

#endif