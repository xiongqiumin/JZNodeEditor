#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include "JZNodeMemberSelectDialog.h"
#include "ui_JZNodeMemberSelectDialog.h"
#include "JZNodeObject.h"
#include "JZNodeTypeHelper.h"
#include "JZClassItem.h"
#include "JZProject.h"

//JZNodeMemberSelectDialog
JZNodeMemberSelectDialog::JZNodeMemberSelectDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::JZNodeMemberSelectDialog();
    ui->setupUi(this);

    ui->lineName->setPlaceholderText("请输入或选择类名");
    ui->treeWidget->setHeaderHidden(true);
}

JZNodeMemberSelectDialog::~JZNodeMemberSelectDialog()
{
    delete ui;
}

QString JZNodeMemberSelectDialog::className()
{
    return m_className;
}

QStringList JZNodeMemberSelectDialog::paramList()
{
    QStringList result;
    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        result << ui->listWidget->item(i)->text();
    }
    return result;
}

void JZNodeMemberSelectDialog::init(JZProject *project, QString class_name)
{
    m_project = project;
    m_className = class_name;
    ui->lineName->setText(class_name);
    updateTree();    
}

void JZNodeMemberSelectDialog::updateTree()
{
    QString className = ui->lineName->text();
    if (m_className == className)
        return;

    auto obj_inst = editorObjectManager();
    m_className = className;
    ui->treeWidget->clear();
    ui->listWidget->clear();
    auto def = obj_inst->meta(className);
    if (def)
    {
        QTreeWidgetItem *item = createTreeItem(className, def->id);
        ui->treeWidget->addTopLevelItem(item);
        ui->treeWidget->expandAll();
    }
}

QTreeWidgetItem *JZNodeMemberSelectDialog::createTreeItem(QString name, int type)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);

    auto obj_inst = editorObjectManager();
    if (JZNodeType::isObject(type))
    {
        auto def = obj_inst->meta(type);
        auto super = def->super();
        if (super && super->paramList(true).size() > 0)
        {
            auto super_item = createTreeItem(super->className, super->id);
            item->addChild(super_item);
        }

        auto list = def->paramList(false);
        for (int i = 0; i < list.size(); i++)
        {
            auto param = def->param(list[i]);
            //           auto sub_item = createTreeItem(list[i], param->dataType);
            //           item->addChild(sub_item);
        }
    }

    return item;
}

void JZNodeMemberSelectDialog::on_lineName_returnPressed()
{
    updateTree();
}

void JZNodeMemberSelectDialog::on_btnSelect_clicked()
{
    auto obj_inst = editorObjectManager();
    QString className = ui->lineName->text();
    auto def = obj_inst->meta(className);

    JZNodeTypeDialog dialog(this);
    if (def)
        dialog.setDataType(className);
    if (dialog.exec() != QDialog::Accepted)
        return;

    def = obj_inst->meta(dialog.dataType());
    ui->lineName->setText(def->className);
    updateTree();
}

void JZNodeMemberSelectDialog::on_btnMoveRight_clicked()
{
    auto items = ui->treeWidget->selectedItems();
    for (int i = 0; i < items.size(); i++)
    {
        QString text = items[i]->text(0);
        if (ui->listWidget->findItems(text, Qt::MatchExactly).size() == 0)
            ui->listWidget->addItem(text);
    }
}

void JZNodeMemberSelectDialog::on_btnMoveLeft_clicked()
{
    auto items = ui->listWidget->selectedItems();
    for (int i = 0; i < items.size(); i++)
        delete items[i];
}

void JZNodeMemberSelectDialog::on_btnOk_clicked()
{
    QStringList params;
    int n = ui->listWidget->count();
    for (int i = 0; i < n; i++)
        params << ui->listWidget->item(i)->text();    
}

void JZNodeMemberSelectDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}