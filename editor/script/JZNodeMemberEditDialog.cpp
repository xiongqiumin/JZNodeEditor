#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include "JZNodeMemberEditDialog.h"
#include "ui_JZNodeMemberEditDialog.h"
#include "JZNodeObject.h"
#include "JZNodeTypeDialog.h"
#include "JZClassItem.h"
#include "JZProject.h"

//JZNodeMemberEditDialog
JZNodeMemberEditDialog::JZNodeMemberEditDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::JZNodeMemberEditDialog();
    ui->setupUi(this);

    ui->lineName->setPlaceholderText("请输入或选择类名");
    ui->treeWidget->setHeaderHidden(true);
}

JZNodeMemberEditDialog::~JZNodeMemberEditDialog()
{
    delete ui;
}

QString JZNodeMemberEditDialog::className()
{
    return m_className;
}

QStringList JZNodeMemberEditDialog::paramList()
{
    QStringList result;
    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        result << ui->listWidget->item(i)->text();
    }
    return result;
}

void JZNodeMemberEditDialog::init(JZNode *node)
{    
    m_node = (JZNodeAbstractMember*)node;
    QString class_name = m_node->className();
    if (class_name.isEmpty())
    {
        auto project = node->file()->project();
        auto class_file = project->getClass(node->file());
        if (class_file)
            class_name = class_file->className();
    }
    ui->lineName->setText(class_name);
    updateTree();
    ui->listWidget->addItems(m_node->members());
}

void JZNodeMemberEditDialog::updateTree()
{
    QString className = ui->lineName->text();
    if (m_className == className)
        return;

    m_className = className;
    ui->treeWidget->clear();
    ui->listWidget->clear();    
    auto def = JZNodeObjectManager::instance()->meta(className);
    if (def)
    {
        QTreeWidgetItem *item = createTreeItem(className, def->id);
        ui->treeWidget->addTopLevelItem(item);
        ui->treeWidget->expandAll();
    }
}

QTreeWidgetItem *JZNodeMemberEditDialog::createTreeItem(QString name, int type)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);

    if (JZNodeType::isObject(type))
    {        
        auto def = JZNodeObjectManager::instance()->meta(type);
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
            auto sub_item = createTreeItem(list[i], param->dataType);
            item->addChild(sub_item);
        }
    }

    return item;
}

void JZNodeMemberEditDialog::on_lineName_returnPressed()
{
    updateTree();
}

void JZNodeMemberEditDialog::on_btnSelect_clicked()
{
    QString className = ui->lineName->text();
    auto def = JZNodeObjectManager::instance()->meta(className);

    JZNodeTypeDialog dialog(this);
    if (def)
        dialog.setDataType(def->id);
    if (dialog.exec() != QDialog::Accepted)
        return;
    
    def = JZNodeObjectManager::instance()->meta(dialog.dataType());
    ui->lineName->setText(def->className);
    updateTree();
}

void JZNodeMemberEditDialog::on_btnMoveRight_clicked()
{
    auto items = ui->treeWidget->selectedItems();
    for (int i = 0; i < items.size(); i++)
    {
        QString text = items[i]->text(0);
        if(ui->listWidget->findItems(text,Qt::MatchExactly).size() == 0)
            ui->listWidget->addItem(text);
    }
}

void JZNodeMemberEditDialog::on_btnMoveLeft_clicked()
{
    auto items = ui->listWidget->selectedItems();
    for (int i = 0; i < items.size(); i++)
        delete items[i];
}

void JZNodeMemberEditDialog::on_btnOk_clicked()
{    
    QStringList params;
    int n = ui->listWidget->count();
    for (int i = 0; i < n; i++)
        params << ui->listWidget->item(i)->text();

    if (m_className != m_node->className() || params != m_node->members())
    {
        m_node->setMember(m_className, params);
        QDialog::accept();
    }
    else
    {
        QDialog::reject();
    }
}

void JZNodeMemberEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}