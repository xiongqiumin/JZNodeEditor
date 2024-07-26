#include <QHBoxLayout>
#include <QMessageBox>
#include "JZNodeExprEditDialog.h"
#include "JZNodeExpression.h"

//JZNodeExprEditDialog
JZNodeExprEditDialog::JZNodeExprEditDialog(QWidget *parent)
    : JZBaseDialog(parent)
{
    m_edit = new QPlainTextEdit();

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->addWidget(m_edit);
    m_mainWidget->setLayout(l);
}

JZNodeExprEditDialog::~JZNodeExprEditDialog()
{
}

void JZNodeExprEditDialog::setExpr(QString expr)
{
    m_edit->setPlainText(expr);
}

QString JZNodeExprEditDialog::expr()
{
    return m_edit->toPlainText();
}

bool JZNodeExprEditDialog::onOk()
{
    JZNodeExpression expr_node;
    QString error;
    if(!expr_node.setExpr(expr(),error))
    {
        QMessageBox::information(this,"",error);
        return false;
    }

    return true;
}