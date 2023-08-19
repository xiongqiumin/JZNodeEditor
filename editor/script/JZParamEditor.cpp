#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <JZNodeType.h>
#include <QComboBox>

#include "JZParamEditor.h"
#include "ui_JZParamEditor.h"
#include "JZProject.h"
#include "JZNodeTypeDialog.h"

//JZParamEditorCommand
JZParamEditorCommand::JZParamEditorCommand(JZParamEditor *editor, int type)
{
    m_editor = editor;
    command = type;
}

void JZParamEditorCommand::redo()
{
    if(command == NewParam)
    {
        m_editor->addParam(newName,newType);
    }
    else if (command == RemoveParam)
    {
        m_editor->removeParam(newName);
    }
    else if (command == Rename)
    {
        m_editor->renameParam(oldName, newName);
    }
    else if (command == SetType) 
    {
        m_editor->setParamType(newName, newType);
    }
}

void JZParamEditorCommand::undo()
{
    if (command == NewParam)
    {
        m_editor->removeParam(newName);
    }
    else if (command == RemoveParam)
    {
        m_editor->addParam(newName,newType);
    }
    else if (command == Rename)
    {
        m_editor->renameParam(newName, oldName);
    }
    else if (command == SetType)
    {
        m_editor->setParamType(newName, oldType);
    }
}

int JZParamEditorCommand::id() const
{
    return -1;
}

bool JZParamEditorCommand::mergeWith(const QUndoCommand *command)
{
    return false;
}

//JZParamEditor
JZParamEditor::JZParamEditor()
    :ui(new Ui::JZParamEditor())
{
    ui->setupUi(this);    
    m_widgetCount = 0;
    m_table = ui->tableWidget;

    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"名称","类型","默认值"});
    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    
    connect(m_table, &QTableWidget::itemChanged, this, &JZParamEditor::onItemChanged);
    connect(&m_commandStack, &QUndoStack::cleanChanged, this, &JZParamEditor::onCleanChanged);
    connect(&m_commandStack, &QUndoStack::canRedoChanged, this, &JZParamEditor::redoAvailable);
    connect(&m_commandStack, &QUndoStack::canUndoChanged, this, &JZParamEditor::undoAvailable);
}

JZParamEditor::~JZParamEditor()
{
    delete ui;
}

void JZParamEditor::updateItem(int row,JZParamDefine *def)
{
    QTableWidgetItem *itemName = new QTableWidgetItem(def->name);
    itemName->setData(Qt::UserRole, def->name);
    m_table->setItem(row, 0, itemName);

    if (row < m_widgetCount)
    {
        QString type = JZNodeObjectManager::instance()->getClassName(def->dataType);

        QTableWidgetItem *itemType = new QTableWidgetItem(def->name);        
        itemType->setText(type);
        m_table->setItem(row, 1, itemType);

        itemType->setFlags(itemType->flags() & ~Qt::ItemIsEditable);
        itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
    }
    else
    {
        TypeEditHelp help;
        help.init(def->dataType);

        QComboBox *box = new QComboBox();
        for (int i = 0; i < help.typeNames.size(); i++)
            box->addItem(help.typeNames[i], help.types[i]);
        box->setCurrentIndex(help.index);
        connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));
        m_table->setCellWidget(row, 1, box);
    }
}

void JZParamEditor::open(JZProjectItem *item)
{   
    m_file = dynamic_cast<JZParamFile*>(item);
    m_table->blockSignals(true);
    m_table->clearContents();

    m_widgetCount = 0;
    JZScriptClassFile *class_file = m_project->getClassFile(item);
    if (class_file)
    {
        auto widgets = class_file->uiWidgets();
        m_table->setRowCount(widgets.size());
        m_widgetCount = widgets.size();
        for (int i = 0; i < widgets.size(); i++)
        {            
            updateItem(i, &widgets[i]);
        }        
    }
    
    QStringList list = m_file->variableList();
    m_table->setRowCount(m_widgetCount + list.size());
    for(int i = 0; i < list.size(); i++)
    {
        auto info = m_file->getVariable(list[i]);
        updateItem(m_widgetCount + i, info);
    }
    m_table->blockSignals(false);
}

void JZParamEditor::close()
{

}

void JZParamEditor::save()
{
    m_project->saveItem(m_file);
    m_commandStack.setClean();
}

bool JZParamEditor::isModified()
{
    return !m_commandStack.isClean();
}

void JZParamEditor::onCleanChanged(bool clean)
{
    emit modifyChanged(!clean);
}

void JZParamEditor::onItemChanged(QTableWidgetItem *item)
{
    if (item->column() != 0)
        return;

    QString oldName = item->data(Qt::UserRole).toString();
    QString newName = item->text();
    addRenameCommand(oldName, newName);
}

void JZParamEditor::addNewCommand(QString name, int type)
{
    JZParamEditorCommand *cmd = new JZParamEditorCommand(this, JZParamEditorCommand::NewParam);
    cmd->newName = name;
    cmd->newType = type;
    m_commandStack.push(cmd);

}
void JZParamEditor::addRemoveCommand(QString name)
{
    auto info = m_file->getVariable(name);

    JZParamEditorCommand *cmd = new JZParamEditorCommand(this, JZParamEditorCommand::RemoveParam);
    cmd->newName = name;
    cmd->newType = info->dataType;
    m_commandStack.push(cmd);
}

void JZParamEditor::addRenameCommand(QString oldName, QString newName)
{
    JZParamEditorCommand *cmd = new JZParamEditorCommand(this, JZParamEditorCommand::Rename);
    cmd->newName = newName;
    cmd->oldName = oldName;
    m_commandStack.push(cmd);
}

void JZParamEditor::addSetTypeCommand(QString name, int newType)
{
    auto info = m_file->getVariable(name);

    JZParamEditorCommand *cmd = new JZParamEditorCommand(this, JZParamEditorCommand::SetType);
    cmd->newName = name;
    cmd->oldType = info->dataType;
    cmd->newType = newType;
    m_commandStack.push(cmd);
}

int JZParamEditor::rowIndex(QComboBox *box)
{
    for (int i = 0; i < m_table->rowCount(); i++)
    {
        if (m_table->cellWidget(i,1) == box)
            return i;
    }
    return -1;
}

int JZParamEditor::rowIndex(QString name)
{
    for (int i = 0; i < m_table->rowCount(); i++)
    {
        if (m_table->item(i, 0)->data(Qt::UserRole).toString() == name)
            return i;
    }
    return -1;
}

void JZParamEditor::addParam(QString name, int dataType)
{
    int row = m_table->rowCount();
    m_table->setRowCount(row + 1);
    m_file->addVariable(name, dataType);

    m_table->blockSignals(true);
    updateItem(row, m_file->getVariable(name));
    m_table->blockSignals(false);
}

void JZParamEditor::removeParam(QString name)
{
    m_file->removeVariable(name);
    int row = rowIndex(name);    
    m_table->removeRow(row);
}

void JZParamEditor::renameParam(QString oldName, QString newName)
{
    m_file->renameVariable(oldName, newName);

    m_table->blockSignals(true);
    int row = rowIndex(oldName);
    m_table->item(row, 0)->setText(newName);
    m_table->item(row, 0)->setData(Qt::UserRole, newName);
    m_table->blockSignals(false);
}

void JZParamEditor::setParamType(QString name, int dataType)
{
    m_file->setVariableType(name, dataType);

    auto def = m_file->getVariable(name);
    int row = rowIndex(name);
    m_table->blockSignals(true);
    updateItem(row, def);
    m_table->blockSignals(false);
}

void JZParamEditor::on_btnAdd_clicked()
{        
    JZScriptClassFile *class_file = getClassFile(m_file);

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
    addNewCommand(name, Type_int);    
}

void JZParamEditor::on_btnRemove_clicked()
{
    int index = m_table->currentRow();
    if (index == -1 || index < m_widgetCount)
        return;

    QString name = m_table->item(index, 0)->text();
    addRemoveCommand(name);
}

void JZParamEditor::onTypeChanged(int index)
{
    QComboBox *box = qobject_cast<QComboBox*>(sender());
    if (index < box->count() - 1) 
    {
        return;
    }

    JZNodeTypeDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)    
        return;
    
    int row = rowIndex(box);
    QString name = m_table->item(row,0)->text();
    int dataType = dialog.dataType();
    addSetTypeCommand(name, dataType);
}