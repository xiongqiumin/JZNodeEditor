#include "JZNodeTypeDialog.h"
#include "ui_JZNodeTypeDialog.h"

//JZNodeTypeDialog
JZNodeTypeDialog::JZNodeTypeDialog(QWidget *p)
    :QDialog(p)
{
    ui = new Ui::JZNodeTypeDialog();
    ui->setupUi(this);
}

JZNodeTypeDialog::~JZNodeTypeDialog()
{
    delete ui;
}

int JZNodeTypeDialog::dataType()
{
    return Type_none;
}

void JZNodeTypeDialog::on_btnOk_clicked()
{
    QDialog::accept();
}

void JZNodeTypeDialog::on_btnCancel_clicked()
{
    QDialog::rejected();
}

//TypeEditHelp
TypeEditHelp::TypeEditHelp()
{

}

void TypeEditHelp::init(int dataType)
{    
    types = { Type_bool,Type_int,Type_double,Type_string };
    index = types.indexOf(dataType);
    if (index == -1)
    {
        types << dataType;
        index = types.size() - 1;
    }
    typeNames.clear();
    for (int i = 0; i < types.size(); i++)
        typeNames << JZNodeType::typeToName(types[i]);
    typeNames << "更多类型...";
}