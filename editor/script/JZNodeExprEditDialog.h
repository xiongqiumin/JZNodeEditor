#ifndef JZNODE_EXPR_EDIT_DIALOG_H_
#define JZNODE_EXPR_EDIT_DIALOG_H_

#include <QDialog>
#include <QPlainTextEdit>
#include "JZBaseDialog.h"

namespace Ui { class JZNodeParamBindEditDialog; }


//JZNodeGroupEditDialog
class JZNodeExprEditDialog : public JZBaseDialog
{
    Q_OBJECT

public:
    JZNodeExprEditDialog(QWidget *p = nullptr);
    ~JZNodeExprEditDialog();

    void setExpr(QString expr);
    QString expr();

protected slots:


protected:
    virtual bool onOk();

    QPlainTextEdit *m_edit;
};

#endif