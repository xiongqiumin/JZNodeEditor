#ifndef JZNODE_PARAM_BIND_EDIT_DIALOG_H_
#define JZNODE_PARAM_BIND_EDIT_DIALOG_H_

#include <QDialog>
#include "UiCommon.h"
#include "JZNode.h"
#include "JZNodeValue.h"

namespace Ui { class JZNodeParamBindEditDialog; }


//JZNodeGroupEditDialog
class JZNodeParamBindEditDialog : public QDialog
{
    Q_OBJECT

public:
    JZNodeParamBindEditDialog(QWidget *p = nullptr);
    ~JZNodeParamBindEditDialog();

protected slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

protected:
    Ui::JZNodeParamBindEditDialog *ui;
};

#endif