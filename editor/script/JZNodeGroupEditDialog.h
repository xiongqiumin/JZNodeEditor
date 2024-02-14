#ifndef JZNODE_GROUP_EDIT_DIALOG_H_
#define JZNODE_GROUP_EDIT_DIALOG_H_

#include <QDialog>
#include "UiCommon.h"
#include "JZNode.h"

namespace Ui { class JZNodeGroupEditDialog; }


//JZNodeGroupEditDialog
class JZNodeGroupEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    JZNodeGroupEditDialog(QWidget *p = nullptr);
    ~JZNodeGroupEditDialog();
    
    void setGroup(JZNodeGroup group);
    JZNodeGroup group();

protected slots:    
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

protected:
    JZNodeGroup m_group;

    Ui::JZNodeGroupEditDialog *ui;
};

#endif
