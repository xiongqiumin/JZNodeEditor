#include "JZNodeTypeDialog.h"
#include "ui_JZNodeTypeDialog.h"
#include "JZNodeObject.h"
#include <QComboBox>

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

void TypeEditHelp::update(QComboBox *box)
{
    box->blockSignals(true);
    box->clear();
    for (int i = 0; i < typeNames.size(); i++)
        box->addItem(typeNames[i], types[i]);
    box->setCurrentIndex(index);
    box->blockSignals(false);
}

//JZNodeTypeDialog
JZNodeTypeDialog::JZNodeTypeDialog(QWidget *p)
    :QDialog(p)
{
    ui = new Ui::JZNodeTypeDialog();
    ui->setupUi(this);
    m_dataType = Type_none;

    ui->treeWidget->setHeaderHidden(true);

    QTreeWidgetItem *item_class = new QTreeWidgetItem();
    item_class->setText(0, "object");
    ui->treeWidget->addTopLevelItem(item_class);

    auto inst = JZNodeObjectManager::instance();
    auto list = inst->getClassList();
    for (int i = 0; i < list.size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        int id = inst->getClassId(list[i]);
        item->setText(0, list[i]);
        item->setData(0, Qt::UserRole, id);
        item_class->addChild(item);
    }

    //item_enum
    QTreeWidgetItem *item_enum = new QTreeWidgetItem();
    item_enum->setText(0, "enum");
    ui->treeWidget->addTopLevelItem(item_enum);

    auto enum_list = inst->getEnumList();
    for (int i = 0; i < enum_list.size(); i++)
    {        
        QTreeWidgetItem *item = new QTreeWidgetItem();
        int id = inst->getEnumId(enum_list[i]);
        item->setText(0, enum_list[i]);
        item->setData(0, Qt::UserRole, id);
        item_enum->addChild(item);
    }    

    ui->treeWidget->expandAll();
}

JZNodeTypeDialog::~JZNodeTypeDialog()
{
    delete ui;
}

void JZNodeTypeDialog::setDataType(int dataType)
{
    if (JZNodeType::isBase(dataType))
        return;

    QString className = JZNodeType::typeToName(dataType);
    auto list = ui->treeWidget->findItems(className, Qt::MatchExactly);
    if (list.size() >= 0)
    {
        ui->treeWidget->scrollToItem(list[0]);        
        ui->treeWidget->setItemSelected(list[0], true);
    }
}

int JZNodeTypeDialog::dataType()
{
    return m_dataType;
}

void JZNodeTypeDialog::on_lineClassName_returnPressed()
{
    auto line = ui->lineClassName->text();    
    UiHelper::treeFilter(ui->treeWidget->invisibleRootItem(), line);
}

void JZNodeTypeDialog::on_btnOk_clicked()
{
    auto item = ui->treeWidget->currentItem();
    if (!item || !item->data(0,Qt::UserRole).isValid())
    {
        QMessageBox::information(this, "", "请先选择一项");
        return;
    }

    m_dataType = item->data(0, Qt::UserRole).toInt();
    QDialog::accept();
}

void JZNodeTypeDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}