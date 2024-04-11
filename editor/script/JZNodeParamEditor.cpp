#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <JZNodeType.h>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QKeyEvent>
#include <QPushButton>

#include "JZNodeParamEditor.h"
#include "ui_JZNodeParamEditor.h"
#include "JZProject.h"
#include "JZNodeTypeHelper.h"
#include "JZBaseDialog.h"
#include "JZNodeParamBindEditDialog.h"
#include "JZNodeParamWidget.h"

static int bindDir(QString text)
{
    if (text == "DataToUi")
        return JZNodeParamBind::DataToUi;
    else if (text == "UiToData")
        return JZNodeParamBind::UiToData;
    else
        return JZNodeParamBind::Duplex;
}

static QString bindText(int dir)
{
    if (dir == JZNodeParamBind::DataToUi)
        return "DataToUi";
    else if (dir == JZNodeParamBind::UiToData)
        return "UiToData";
    else
        return "Duplex";
}

//
class ValueItemDelegate : public QStyledItemDelegate
{
public:
    ValueItemDelegate(QObject *parent)
        :QStyledItemDelegate(parent)
    {
    }

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {        
        auto item = m_table->item(index.row(), index.column());
        QString type_text = m_table->item(index.row(), 1)->text();        
        int dataType = JZNodeType::nameToType(type_text);
        if (dataType == Type_none)
            return nullptr;

        auto edit = new JZNodeParamValueWidget(parent);
        edit->setDataType({ dataType });
        edit->setValue(option.text);
        return edit;
    }

    void setModelData(QWidget *editor,
        QAbstractItemModel *model,
        const QModelIndex &index) const override
    {
        auto edit = qobject_cast<JZNodeParamValueWidget*>(editor);
        model->setData(index, edit->value());
    }

    void setTable(QTableWidget *table)
    {
        m_table = table;
    }

    QTableWidget *m_table;
};

class BindItemDelegate : public QStyledItemDelegate
{
public:
    BindItemDelegate(QObject *parent)
        :QStyledItemDelegate(parent)
    {

    }

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        JZNodeParamBindEditDialog *dlg = new JZNodeParamBindEditDialog(parent);
        QString name = index.model()->data(index.siblingAtColumn(0)).toString();
        dlg->init(m_editor->classItem(), name);
        return dlg;
    }

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        auto dlg = qobject_cast<JZNodeParamBindEditDialog*>(editor);
        QString name = index.model()->data(index.siblingAtColumn(0)).toString();
        QString text = index.model()->data(index).toString();        
        if (text.size() > 0)
        {
            QStringList values = text.split("|");

            JZNodeParamBind bind;
            bind.variable = name;
            bind.widget = values[0];
            bind.dir = bindDir(values[1]);
            dlg->setParamBind(bind);
        }
    }

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        auto dlg = qobject_cast<JZNodeParamBindEditDialog*>(editor);       
        if (dlg->result() != QDialog::Accepted)
            return;

        auto bind = dlg->paramBind();
        if (bind.widget.isEmpty())
        {
            model->setData(index, QString());
        }
        else
        {
            QString text = bind.widget + "|" + bindText(bind.dir);
            model->setData(index, text);
        }
    }

    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        auto pt = editor->parentWidget()->mapToGlobal(option.rect.topLeft());
        pt.ry() -= 30;
        editor->move(pt);
    }

    JZNodeParamEditor *m_editor;
};

//JZNodeParamEditorCommand
JZNodeParamEditorCommand::JZNodeParamEditorCommand(JZNodeParamEditor *editor, int type)
{
    m_editor = editor;
    command = type;
}

void JZNodeParamEditorCommand::redo()
{
    if(command == NewParam)
    {
        m_editor->addParam(newParam);
    }
    else if (command == RemoveParam)
    {
        m_editor->removeParam(oldParam.name);
    }
    else if (command == Rename)
    {
        m_editor->renameParam(oldParam.name, newParam.name);
    }
    else if (command == Change) 
    {
        m_editor->changeParam(newParam.name, newParam);
    }
    else if (command == Bind)
    {
        m_editor->bindParam(newBind.variable, newBind);
    }
}

void JZNodeParamEditorCommand::undo()
{
    if (command == NewParam)
    {
        m_editor->removeParam(newParam.name);
    }
    else if (command == RemoveParam)
    {
        m_editor->addParam(oldParam);
    }
    else if (command == Rename)
    {
        m_editor->renameParam(newParam.name, oldParam.name);
    }
    else if (command == Change)
    {
        m_editor->changeParam(oldParam.name, oldParam);
    }
    else if (command == Bind)
    {
        m_editor->bindParam(oldBind.variable, oldBind);
    }
}

int JZNodeParamEditorCommand::id() const
{
    return -1;
}

bool JZNodeParamEditorCommand::mergeWith(const QUndoCommand *command)
{
    return false;
}

//JZNodeParamEditor
JZNodeParamEditor::JZNodeParamEditor()
    :ui(new Ui::JZNodeParamEditor())
{
    ui->setupUi(this);    
    m_widgetCount = 0;
    m_isClass = false;
    m_table = ui->tableWidget;

    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"名称","类型","默认值","参数绑定"});
    m_table->setSelectionBehavior(QTableWidget::SelectItems);
    m_table->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);    

    ValueItemDelegate *delegate = new ValueItemDelegate(this);
    delegate->setTable(m_table);
    m_table->setItemDelegateForColumn(2, delegate);    

    BindItemDelegate *bind_delegate = new BindItemDelegate(this);    
    bind_delegate->m_editor = this;
    m_table->setItemDelegateForColumn(3, bind_delegate);

    TypeItemDelegate *type_delegate = new TypeItemDelegate(this);    
    m_table->setItemDelegateForColumn(1, type_delegate);
    
    connect(m_table, &QTableWidget::itemChanged, this, &JZNodeParamEditor::onItemChanged);
    connect(&m_commandStack, &QUndoStack::cleanChanged, this, &JZNodeParamEditor::onCleanChanged);
    connect(&m_commandStack, &QUndoStack::canRedoChanged, this, &JZNodeParamEditor::redoAvailable);
    connect(&m_commandStack, &QUndoStack::canUndoChanged, this, &JZNodeParamEditor::undoAvailable);
}

JZNodeParamEditor::~JZNodeParamEditor()
{
    delete ui;
}

JZScriptClassItem *JZNodeParamEditor::classItem()
{
    return m_project->getItemClass(m_file);
}

void JZNodeParamEditor::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return)
    {
        auto item = m_table->currentItem();
        if(item && item->flags() & Qt::ItemIsEditable)
        {
            m_table->editItem(item);
            e->accept();
            return;
        }
    }
    QWidget::keyPressEvent(e);
}

void JZNodeParamEditor::updateItem(int row, const JZParamDefine *def)
{
    QTableWidgetItem *itemName = new QTableWidgetItem(def->name);
    itemName->setData(Qt::UserRole, def->name);
    m_table->setItem(row, 0, itemName);    
    
    QTableWidgetItem *itemType = new QTableWidgetItem(def->name);        
    itemType->setText(def->type);
    m_table->setItem(row, 1, itemType);

    QTableWidgetItem *itemValue = new QTableWidgetItem();            
    m_table->setItem(row, 2, itemValue);
    itemValue->setText(def->initValue());
    
    QTableWidgetItem *itemBind = new QTableWidgetItem();
    m_table->setItem(row, 3, itemBind);        

    auto bind = m_file->bindVariable(def->name);
    if (bind)
    {
        itemBind->setText(bind->widget + "|" + bindText(bind->dir));
    }

    if (row < m_widgetCount)
    {
        itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
        itemType->setFlags(itemType->flags() & ~Qt::ItemIsEditable);        
        itemValue->setFlags(itemValue->flags() & ~Qt::ItemIsEditable);
    }
    else
    {
        if(!JZNodeType::isBaseOrEnum(def->dataType()))
            itemValue->setFlags(itemValue->flags() & ~Qt::ItemIsEditable);
    }

    if (JZNodeType::isInherits(def->type, "Widget"))
        itemBind->setFlags(itemBind->flags() & ~Qt::ItemIsEditable);
}

void JZNodeParamEditor::open(JZProjectItem *item)
{   
    m_file = dynamic_cast<JZParamItem*>(item);
        
    m_table->blockSignals(true);
    m_table->clearContents();

    m_widgetCount = 0;
    JZScriptClassItem *class_file = m_project->getItemClass(item);
    if (class_file)
    {
        m_table->setColumnHidden(3, false);
        m_isClass = true;

        auto widgets = class_file->uiWidgets();
        m_table->setRowCount(widgets.size());
        m_widgetCount = widgets.size();
        for (int i = 0; i < widgets.size(); i++)
        {            
            updateItem(i, &widgets[i]);
        }        
    }
    else
    {
        m_table->setColumnHidden(3, true);
        m_isClass = false;
    }
    
    QStringList list = m_file->variableList();
    m_table->setRowCount(m_widgetCount + list.size());
    for(int i = 0; i < list.size(); i++)
    {
        auto info = m_file->variable(list[i]);
        updateItem(m_widgetCount + i, info);
    }
    m_table->blockSignals(false);
}

void JZNodeParamEditor::close()
{

}

void JZNodeParamEditor::save()
{
    m_project->saveItem(m_file);
    m_commandStack.setClean();
}

bool JZNodeParamEditor::isModified()
{
    return !m_commandStack.isClean();
}

void JZNodeParamEditor::redo()
{
    m_commandStack.redo();
}

void JZNodeParamEditor::undo()
{
    m_commandStack.undo();
}

void JZNodeParamEditor::onCleanChanged(bool clean)
{
    emit modifyChanged(!clean);
}

void JZNodeParamEditor::onItemChanged(QTableWidgetItem *item)
{
    QString varName = m_table->item(item->row(), 0)->data(Qt::UserRole).toString();
    if (item->column() == 0)
    {        
        QString newName = item->text();
        addRenameCommand(varName, newName);
    }
    else if (item->column() == 2)
    {
        QString value = item->text();
        JZParamDefine info = *m_file->variable(varName);
        info.value = value;
        addChangeCommand(varName, info);
    }
    else if (item->column() == 3)
    {
        QStringList value = item->text().split("|");        
        JZNodeParamBind info;
        info.variable = varName;
        if (value.size() == 2)
        {            
            info.widget = value[0];
            info.dir = bindDir(value[1]);
        }
        addBindCommand(varName, info);
    }
}

void JZNodeParamEditor::addNewCommand(QString name, QString type)
{
    JZParamDefine param;
    param.name = name;
    param.type = type;

    JZNodeParamEditorCommand *cmd = new JZNodeParamEditorCommand(this, JZNodeParamEditorCommand::NewParam);
    cmd->newParam = param;
    m_commandStack.push(cmd);

}
void JZNodeParamEditor::addRemoveCommand(QString name)
{
    auto info = m_file->variable(name);

    JZNodeParamEditorCommand *cmd = new JZNodeParamEditorCommand(this, JZNodeParamEditorCommand::RemoveParam);    
    cmd->oldParam = *info;
    m_commandStack.push(cmd);
}

void JZNodeParamEditor::addRenameCommand(QString oldName, QString newName)
{
    auto info = m_file->variable(oldName);

    JZNodeParamEditorCommand *cmd = new JZNodeParamEditorCommand(this, JZNodeParamEditorCommand::Rename);
    cmd->oldParam = *info;
    cmd->newParam = *info;
    cmd->newParam.name = newName;
    m_commandStack.push(cmd);
}

void JZNodeParamEditor::addChangeCommand(QString name, JZParamDefine define)
{
    auto info = m_file->variable(name);

    JZNodeParamEditorCommand *cmd = new JZNodeParamEditorCommand(this, JZNodeParamEditorCommand::Change);    
    cmd->oldParam = *info;
    cmd->newParam = define;
    m_commandStack.push(cmd);
}

void JZNodeParamEditor::addBindCommand(QString name, JZNodeParamBind define)
{
    auto oldBind = m_file->bindVariable(name);
    if (!oldBind && define.widget.isEmpty())
        return;
    if (oldBind && oldBind->widget == define.widget && oldBind->dir == define.dir)
        return;

    JZNodeParamEditorCommand *cmd = new JZNodeParamEditorCommand(this, JZNodeParamEditorCommand::Bind);
    if(oldBind)
        cmd->oldBind = *oldBind;
    else
        cmd->oldBind.variable = name;
    cmd->newBind = define;
    m_commandStack.push(cmd);
}

int JZNodeParamEditor::rowDataType(int row)
{
    auto box = (QComboBox*)m_table->cellWidget(row, 1);
    return box->currentData().toInt();
}

int JZNodeParamEditor::rowIndex(QComboBox *box)
{
    for (int i = 0; i < m_table->rowCount(); i++)
    {
        if (m_table->cellWidget(i,1) == box)
            return i;
    }
    return -1;
}

int JZNodeParamEditor::rowIndex(QString name)
{
    for (int i = 0; i < m_table->rowCount(); i++)
    {
        if (m_table->item(i, 0)->data(Qt::UserRole).toString() == name)
            return i;
    }
    return -1;
}

void JZNodeParamEditor::addParam(JZParamDefine define)
{
    int row = m_table->rowCount();
    m_table->setRowCount(row + 1);
    m_file->addVariable(define.name, define.type, define.value);

    m_table->blockSignals(true);
    updateItem(row, m_file->variable(define.name));
    m_table->blockSignals(false);
}

void JZNodeParamEditor::removeParam(QString name)
{
    m_file->removeVariable(name);
    int row = rowIndex(name);    
    m_table->removeRow(row);
}

void JZNodeParamEditor::renameParam(QString oldName, QString newName)
{
    auto info = *m_file->variable(oldName);
    info.name = newName;
    m_file->setVariable(oldName, info);

    m_table->blockSignals(true);
    int row = rowIndex(oldName);
    m_table->item(row, 0)->setText(newName);
    m_table->item(row, 0)->setData(Qt::UserRole, newName);
    m_table->blockSignals(false);
}

void JZNodeParamEditor::changeParam(QString name, JZParamDefine define)
{
    m_file->setVariable(name, define);

    auto def = m_file->variable(name);
    int row = rowIndex(name);
    m_table->blockSignals(true);
    updateItem(row, def);
    m_table->blockSignals(false);
}

void JZNodeParamEditor::bindParam(QString name, JZNodeParamBind define)
{
    if (define.widget.isEmpty())
        m_file->removeBind(name);
    else
        m_file->addBind(define);

    auto def = m_file->variable(name);
    int row = rowIndex(name);
    m_table->blockSignals(true);
    updateItem(row, def);
    m_table->blockSignals(false);
}

void JZNodeParamEditor::on_btnAdd_clicked()
{        
    JZScriptClassItem *class_file = getClassFile(m_file);

    QStringList namelist;    
    if (class_file)
        namelist = m_file->variableList();
    else
        namelist = m_project->globalVariableList();
    
    QString name;    
    for(int i = 0;;i++)
    {
        name = "新参数" + QString::number(i + 1);
        if (!namelist.contains(name))
            break;                
    }
    int dataType = Type_int;
    addNewCommand(name, "int");    
}

void JZNodeParamEditor::on_btnRemove_clicked()
{
    int index = m_table->currentRow();
    if (index == -1 || index < m_widgetCount)
        return;

    QString name = m_table->item(index, 0)->text();
    addRemoveCommand(name);
}