#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QKeyEvent>
#include <QPushButton>

#include "JZNodeType.h"
#include "JZNodeParamEditor.h"
#include "ui_JZNodeParamEditor.h"
#include "JZProject.h"
#include "JZNodeTypeHelper.h"
#include "JZBaseDialog.h"
#include "JZNodeParamBindEditDialog.h"
#include "JZNodeParamWidget.h"
#include "JZNodeUtils.h"

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
        edit->initWidget(dataType);
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
        m_editor->bindParam(newBind.widget, newBind);
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
        m_editor->bindParam(oldBind.widget, oldBind);
    }
}

int JZNodeParamEditorCommand::id() const
{
    return -1;
}

bool JZNodeParamEditorCommand::mergeWith(const QUndoCommand *cmd)
{
    return false;
}

//JZNodeParamEditor
JZNodeParamEditor::JZNodeParamEditor()
    :ui(new Ui::JZNodeParamEditor())
{
    ui->setupUi(this);
    m_class = nullptr;

    ui->boxParamType->addItem("成员");
    ui->boxParamType->addItem("控件成员");

    m_table = ui->tableWidget;    
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"名称","类型","初始值"});
    m_table->setSelectionBehavior(QTableWidget::SelectItems);
    m_table->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    m_tableUi = ui->tableWidgetUi;
    m_tableUi->setColumnCount(3);
    m_tableUi->setHorizontalHeaderLabels({ "名称","类型","参数绑定" });
    m_tableUi->setSelectionBehavior(QTableWidget::SelectItems);
    m_tableUi->setEditTriggers(QAbstractItemView::NoEditTriggers);

    TypeItemDelegate *type_delegate = new TypeItemDelegate(this);    
    m_table->setItemDelegateForColumn(1, type_delegate);

    ValueItemDelegate *value_delegate = new ValueItemDelegate(this);
    value_delegate->setTable(m_table);
    m_table->setItemDelegateForColumn(2, value_delegate);        
    
    connect(m_table, &QTableWidget::itemChanged, this, &JZNodeParamEditor::onItemChanged);
    connect(m_tableUi, &QTableWidget::itemChanged, this, &JZNodeParamEditor::onUiItemChanged);
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
    
    QTableWidgetItem *itemType = new QTableWidgetItem(def->type);    
    m_table->setItem(row, 1, itemType);    
}

void JZNodeParamEditor::updateUiItem(int row,const JZParamDefine *def)
{
    QTableWidgetItem *itemName = new QTableWidgetItem(def->name);
    itemName->setData(Qt::UserRole, def->name);
    m_tableUi->setItem(row, 0, itemName);

    QTableWidgetItem *itemType = new QTableWidgetItem(def->type);            
    m_tableUi->setItem(row, 1, itemType);

    //bind
    auto bind = m_file->bindVariable(def->name);

    QWidget *widget = new QWidget();
    QHBoxLayout *l = new QHBoxLayout();
    widget->setLayout(l);
    l->setContentsMargins(3, 3, 3, 3);

    QLineEdit *line = new QLineEdit();
    line->setReadOnly(true);
    if (bind)
        line->setText(bind->variable);
    l->addWidget(line);

    QPushButton *btn = new QPushButton("设置");
    connect(btn, &QPushButton::clicked, this, &JZNodeParamEditor::onParamBind);
    btn->setProperty("rowItem", QVariant::fromValue<void*>(itemName));
    itemName->setData(Qt::UserRole + 1, QVariant::fromValue<void*>(line));
    l->addWidget(btn);
    l->setSpacing(1);

    m_tableUi->setCellWidget(row, 2, widget);
}

void JZNodeParamEditor::open(JZProjectItem *item)
{   
    m_file = dynamic_cast<JZParamItem*>(item);
        
    m_table->blockSignals(true);
    m_table->clearContents();
    
    m_class = m_project->getItemClass(item);
    if (m_class)
    {
        auto widgets = m_class->uiWidgets();
        m_tableUi->setRowCount(widgets.size());        
        for (int i = 0; i < widgets.size(); i++)
        {            
            auto &def = widgets[i];
            updateUiItem(i,&def);
        }        
    }
    
    QStringList list = m_file->variableList();
    m_table->setRowCount(list.size());
    for(int i = 0; i < list.size(); i++)
    {
        auto info = m_file->variable(list[i]);
        updateItem(i, info);
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
    int row = item->row();
    QString varName = m_table->item(row, 0)->data(Qt::UserRole).toString();
    if (item->column() == 0)
    {        
        QString newName = item->text();
        addRenameCommand(varName, newName);
    }
    else
    {
        QString value = item->text();
        JZParamDefine info = *m_file->variable(varName);
        info.type = m_table->item(row,1)->text();
        info.value = m_table->item(row,2)->text();
        addChangeCommand(varName, info);
    }
}

void JZNodeParamEditor::onUiItemChanged(QTableWidgetItem *item)
{
    QString varName = m_tableUi->item(item->row(), 0)->data(Qt::UserRole).toString();
    if (item->column() == 2)
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

int JZNodeParamEditor::rowIndexUi(QString name)
{
    for (int i = 0; i < m_tableUi->rowCount(); i++)
    {
        if (m_tableUi->item(i, 0)->data(Qt::UserRole).toString() == name)
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
    if (define.variable.isEmpty())
        m_file->removeBind(name);
    else
        m_file->addBind(define);

    auto def = m_file->variable(name);
    int row = rowIndexUi(name);
    m_tableUi->blockSignals(true);
    
    auto item = m_tableUi->item(row, 0);
    auto line = (QLineEdit*)item->data(Qt::UserRole + 1).value<void*>();
    line->setText(define.variable);

    m_tableUi->blockSignals(false);
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
    if (index == -1)
        return;

    QString name = m_table->item(index, 0)->text();
    addRemoveCommand(name);
}

void JZNodeParamEditor::on_boxParamType_currentIndexChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void JZNodeParamEditor::onParamBind()
{
    auto btn = qobject_cast<QPushButton*>(sender());

    auto item = (QTableWidgetItem*)btn->property("rowItem").value<void*>();    
    auto name = item->data(Qt::UserRole).toString();
    m_file->variable(name);

    auto def = m_class->uiWidgets()[item->row()];
    auto bind = m_file->bindVariable(def.name);

    JZNodeParamBindEditDialog dlg(this);
    dlg.init(def.type);
    if(bind)
        dlg.setParamBind(*bind);
    else
    {
        JZNodeParamBind b;
        b.widget = def.name;
        dlg.setParamBind(b);
    }

    if (dlg.exec() != JZNodeParamBindEditDialog::Accepted)
        return;

    addBindCommand(def.name, dlg.paramBind());
}

void JZNodeParamEditor::navigate(QUrl url)
{
    auto jz_url = fromQUrl(url);
    if (jz_url.args["type"] == "ui")    
        ui->boxParamType->setCurrentIndex(1);
    else
        ui->boxParamType->setCurrentIndex(0);
}