#include "JZParamEditor.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <JZNodeType.h>

JZParamEditor::JZParamEditor()
{
    QVBoxLayout *layout = new QVBoxLayout();
    m_table = new QTableWidget();
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"名称","类型","绑定控件","方向"});
    layout->addWidget(m_table);
    this->setLayout(layout);
}

JZParamEditor::~JZParamEditor()
{

}

void JZParamEditor::open(JZProjectItem *item)
{
    m_table->clearContents();

    JZParamFile *file = dynamic_cast<JZParamFile*>(item);
    QStringList list = file->variableList();
    m_table->setRowCount(list.size());
    for(int i = 0; i < list.size(); i++)
    {
        auto info = file->getVariable(list[i]);
        QTableWidgetItem *itemName = new QTableWidgetItem(info->name);
        QTableWidgetItem *itemType = new QTableWidgetItem(JZNodeType::typeToName(info->dataType));
        m_table->setItem(i,0,itemName);
        m_table->setItem(i,1,itemType);
    }
}

void JZParamEditor::close()
{

}

void JZParamEditor::save()
{

}
