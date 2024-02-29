#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include "JZNodeFunctionManager.h"
#include "JZNodeFuctionEditDialog.h"
#include "ui_JZNodeFuctionEditDialog.h"
#include "JZNodeTypeHelper.h"

enum {
    Item_name = Qt::UserRole,    
    Item_type,
};


//JZNodeFuctionEditDialog
JZNodeFuctionEditDialog::JZNodeFuctionEditDialog(QWidget *parent)
    : QDialog(parent)
{
    m_newFunction = false;

    ui = new Ui::JZNodeFuctionEditDialog();
    ui->setupUi(this);                   

    TypeItemDelegate *type_delegate1 = new TypeItemDelegate(this);
    ui->tableIn->setItemDelegateForColumn(1, type_delegate1);

    TypeItemDelegate *type_delegate2 = new TypeItemDelegate(this);
    ui->tableOut->setItemDelegateForColumn(1, type_delegate2);

    ui->tableIn->setColumnCount(2);
    ui->tableIn->setHorizontalHeaderLabels({ "名称","类型"});

    ui->tableOut->setColumnCount(2);
    ui->tableOut->setHorizontalHeaderLabels({ "名称","类型"});
}

JZNodeFuctionEditDialog::~JZNodeFuctionEditDialog()
{
    delete ui;
}

void JZNodeFuctionEditDialog::init()
{    
    dataToUi();
}

void JZNodeFuctionEditDialog::setFunctionInfo(FunctionDefine info, bool newFunction)
{
    m_functionDefine = info;
    m_newFunction = newFunction;
}

FunctionDefine JZNodeFuctionEditDialog::functionInfo()
{
    return m_functionDefine;
}

void JZNodeFuctionEditDialog::on_btnInUp_clicked()
{
    if (isMemberFunction() && ui->tableIn->currentRow() == 1)
        return;

    up(ui->tableIn);
}

void JZNodeFuctionEditDialog::on_btnInDown_clicked()
{
    if (isMemberFunction() && ui->tableIn->currentRow() == 0)
        return;

    down(ui->tableIn);
}

void JZNodeFuctionEditDialog::on_btnInAdd_clicked()
{
    add(ui->tableIn);
}

void JZNodeFuctionEditDialog::on_btnInRemove_clicked()
{
    if (isMemberFunction() && ui->tableIn->currentRow() == 0)
        return;

    remove(ui->tableIn);
}

void JZNodeFuctionEditDialog::on_btnOutUp_clicked()
{
    up(ui->tableOut);
}

void JZNodeFuctionEditDialog::on_btnOutDown_clicked()
{
    down(ui->tableOut);
}

void JZNodeFuctionEditDialog::on_btnOutAdd_clicked()
{
    add(ui->tableOut);
}

void JZNodeFuctionEditDialog::on_btnOutRemove_clicked()
{
    remove(ui->tableOut);
}

void JZNodeFuctionEditDialog::on_btnReset_clicked()
{
    if(QMessageBox::information(this,"","是否重置",QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        dataToUi();
}

void JZNodeFuctionEditDialog::on_btnOk_clicked()
{
    uiToData();

    if (m_newFunction)
    {
        auto func = JZNodeFunctionManager::instance()->function(m_functionDefine.fullName());
        if (func)
        {
            QMessageBox::information(this, "", "同名函数已存在");
            return;
        }
    }

    QDialog::accept();
}

void JZNodeFuctionEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}

bool JZNodeFuctionEditDialog::isMemberFunction()
{
    return !m_functionDefine.className.isEmpty();
}

QStringList JZNodeFuctionEditDialog::localVarList()
{
    QStringList names;
    names << "this";
    int count = ui->tableIn->rowCount();
    for (int i = 0; i < count; i++)
        names << ui->tableIn->item(i, 0)->text();
    return names;
}

void JZNodeFuctionEditDialog::add(QTableWidget *table)
{
    auto locals = localVarList();
    QString base = (table == ui->tableIn) ? "input" : "output";
    QString name;
    for (int i = 0;; i++)
    {
        name = base + QString::number(i + 1);
        if (!locals.contains(name))
            break;
    }
    int dataType = Type_int;
    addRow(table,name, "int");
}

void JZNodeFuctionEditDialog::remove(QTableWidget *table)
{
    table->removeRow(table->currentRow());
}

void JZNodeFuctionEditDialog::up(QTableWidget *table)
{
    int row = table->currentRow();
    if (row >= 1)
    {
        swap(table, row, row - 1);
    }
}

void JZNodeFuctionEditDialog::down(QTableWidget *table)
{
    int row = table->currentRow();
    if (row >=0 && row < table->rowCount() - 1)
    {
        swap(table, row, row + 1);
    }
}

void JZNodeFuctionEditDialog::swap(QTableWidget *table, int row0, int row1)
{
    for (int i = 0; i < table->columnCount(); i++)
    {
        auto item0 = table->item(row0, i)->clone();
        auto item1 = table->item(row1, i)->clone();
        table->setItem(row0, i, item1);
        table->setItem(row1, i, item0);
    }
}

void JZNodeFuctionEditDialog::dataToTable(const QList<JZParamDefine> &param, QTableWidget *table)
{
    table->blockSignals(true);
    table->clearContents();    
    table->setRowCount(0);
    for (int i = 0; i < param.size(); i++)
        addRow(table, param[i].name, param[i].type);
    table->blockSignals(false);
}

void JZNodeFuctionEditDialog::tableToData(QTableWidget *table, QList<JZParamDefine> &param)
{
    param.clear();
    int count = table->rowCount();
    for (int row = 0; row < count; row++)
    {        
        JZParamDefine def;
        def.name = table->item(row, 0)->text();
        def.type = table->item(row, 1)->text();
        param.push_back(def);
    }
}

void JZNodeFuctionEditDialog::addRow(QTableWidget *table, QString name, QString dataType)
{
    int row = table->rowCount();
    table->setRowCount(row + 1);

    QTableWidgetItem *itemName = new QTableWidgetItem(name);
    itemName->setData(Item_name, name);    
    table->setItem(row, 0, itemName);
    
    QTableWidgetItem *itemType = new QTableWidgetItem(name);
    itemType->setText(dataType);
    table->setItem(row, 1, itemType);

    if ((table == ui->tableIn && row == 0 && isMemberFunction()))
    {
        itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);                                        
        itemType->setFlags(itemType->flags() & ~Qt::ItemIsEditable);        
    }
}

void JZNodeFuctionEditDialog::dataToUi()
{
    ui->lineName->setText(m_functionDefine.name);
    ui->boxFlow->setChecked(!m_functionDefine.isFlowFunction);
    dataToTable(m_functionDefine.paramIn, ui->tableIn);
    dataToTable(m_functionDefine.paramOut, ui->tableOut);
}

void JZNodeFuctionEditDialog::uiToData()
{
    m_functionDefine.name = ui->lineName->text();
    m_functionDefine.isFlowFunction = !ui->boxFlow->isChecked();
    tableToData(ui->tableIn, m_functionDefine.paramIn);
    tableToData(ui->tableOut, m_functionDefine.paramOut);
}