#include "JZNodeTypeDialog.h"
#include "ui_JZNodeTypeDialog.h"
#include "JZNodeObject.h"

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
    types << Type_none;

}
//JZNodeTypeDialog
JZNodeTypeDialog::JZNodeTypeDialog(QWidget *p)
    :QDialog(p)
{
    ui = new Ui::JZNodeTypeDialog();
    ui->setupUi(this);
    m_dataType = Type_none;

    ui->treeWidget->setHeaderHidden(true);

    auto list = JZNodeObjectManager::instance()->getClassList();
    for (int i = 0; i < list.size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, list[i]);
        ui->treeWidget->addTopLevelItem(item);
    }
}

JZNodeTypeDialog::~JZNodeTypeDialog()
{
    delete ui;
}

int JZNodeTypeDialog::dataType()
{
    return m_dataType;
}

void JZNodeTypeDialog::on_btnOk_clicked()
{
    auto item = ui->treeWidget->currentItem();
    if (!item) 
    {
        QMessageBox::information(this, "", "请先选择一项");
        return;
    }

    m_dataType = JZNodeObjectManager::instance()->getClassId(item->text(0));
    QDialog::accept();
}

void JZNodeTypeDialog::on_btnCancel_clicked()
{
    QDialog::rejected();
}