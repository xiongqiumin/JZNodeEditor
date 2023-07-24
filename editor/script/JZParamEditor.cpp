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
    m_table = ui->tableWidget;

    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"名称","类型","绑定控件","方向"});
    
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

    TypeEditHelp help;
    help.init(def->dataType);

    QComboBox *box = new QComboBox();
    box->addItems(help.typeNames);
    box->setCurrentIndex(help.index);
    connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));
    m_table->setCellWidget(row, 1, box);
}

void JZParamEditor::open(JZProjectItem *item)
{   
    m_file = dynamic_cast<JZParamFile*>(item);
    m_table->blockSignals(true);
    m_table->clearContents();
    
    QStringList list = m_file->variableList();
    m_table->setRowCount(list.size());
    for(int i = 0; i < list.size(); i++)
    {
        auto info = m_file->getVariable(list[i]);
        updateItem(i, info);
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

int JZParamEditor::rowIndex(QString name)
{
    for (int i = 0; i < m_table->rowCount(); i++)
    {
        if (m_table->item(i, 0)->text() == name)
            return i;
    }
    return -1;
}

void JZParamEditor::addParam(QString name, int dataType)
{
    int row = m_table->rowCount();
    m_table->setRowCount(row + 1);
    m_file->addVariable(name, dataType);
    updateItem(row, m_file->getVariable(name));
}

void JZParamEditor::removeParam(QString name)
{
    m_file->removeVariable(name);
    int row = rowIndex(name);
    m_table->removeRow(row);
}

void JZParamEditor::renameParam(QString oldName, QString newName)
{
    int row = rowIndex(oldName);
    m_table->item(row, 0)->setText(newName);
}

void JZParamEditor::setParamType(QString name, int dataType)
{
    auto def = m_file->getVariable(name);
    int row = rowIndex(name);
    updateItem(row, def);
}

void JZParamEditor::on_btnAdd_clicked()
{        
    QString name;    
    for(int i = 0;;i++)
    {
        name = "新参数" + QString::number(i + 1);
        if (!m_project->getVariableInfo(name))
            break;                
    }
    int dataType = Type_int;
    addNewCommand(name, Type_int);    
}

void JZParamEditor::on_btnRemove_clicked()
{
    int index = m_table->currentRow();
    if (index == -1)
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
    {
        return;
    }

    QString name;
    int dataType;
    addSetTypeCommand(name, dataType);
}