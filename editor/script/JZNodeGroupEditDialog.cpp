#include "JZNodeGroupEditDialog.h"
#include "ui_JZNodeGroupEditDialog.h"

//JZNodeGroupEditDialog
JZNodeGroupEditDialog::JZNodeGroupEditDialog(QWidget *p)
    :QDialog(p)
{
    ui = new Ui::JZNodeGroupEditDialog();
    ui->setupUi(this);    
}

JZNodeGroupEditDialog::~JZNodeGroupEditDialog()
{
    delete ui;
}

void JZNodeGroupEditDialog::setGroup(JZNodeGroup group)
{
    m_group = group;
    ui->textEdit->setPlainText(m_group.memo);
}

JZNodeGroup JZNodeGroupEditDialog::group()
{    
    return m_group;
}

void JZNodeGroupEditDialog::on_btnOk_clicked()
{
    m_group.memo = ui->textEdit->toPlainText();
    QDialog::accept();
}

void JZNodeGroupEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}