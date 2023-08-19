#include "JZNodeWatch.h"
#include <QGroupBox>
#include <QPainter>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipBoard>
#include "UiCommon.h"
#include "JZNodeEngine.h"

JZNodeWatch::JZNodeWatch(QWidget *parent)
    :QWidget(parent)
{
    m_running = false;    
    m_newParam = false;
    m_status = Status_none;

    m_view = new QTreeWidget();
    m_view->setColumnCount(3);
    m_view->setHeaderLabels({ "名称","值","类型" });
    m_view->setEditTriggers(QTreeWidget::NoEditTriggers);
    connect(m_view, &QTreeWidget::itemDoubleClicked, this, &JZNodeWatch::onTreeWidgetItemDoubleClicked);
    connect(m_view, &QTreeWidget::itemChanged, this, &JZNodeWatch::onItemChanged);

    QVBoxLayout *sub_layout = new QVBoxLayout();
    sub_layout->addWidget(m_view);
    this->setLayout(sub_layout);
}

JZNodeWatch::~JZNodeWatch()
{

}

void JZNodeWatch::updateStatus()
{
    if (m_running)
    {
        if (statusIsPause(m_status))
            this->setEnabled(true);
        else
            this->setEnabled(false);
    }
    else
    {
        this->setEnabled(true);
        clear();
    }
}

void JZNodeWatch::setRunning(bool isRun)
{
    m_running = isRun;    
    if (!m_running)
        m_status = Status_none;
    updateStatus();
}

void JZNodeWatch::setRuntimeStatus(int status)
{
    m_status = status;
    updateStatus();
}


void JZNodeWatch::onTreeWidgetItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    if (column == 1)
        m_view->editItem(item, column);
}

void JZNodeWatch::onItemChanged(QTreeWidgetItem *item, int column)
{

}

int JZNodeWatch::indexOfItem(QTreeWidgetItem *root, const QString &name)
{
    for (int i = 0; i < root->childCount(); i++)
    {
        auto sub = root->child(i);
        if (sub->text(0) == name)
            return i;
    }
    return -1;
}

void JZNodeWatch::setItem(QTreeWidgetItem *root, int index, const QString &name, const JZNodeDebugParamValue &info)
{
    QString preValue;
    QTreeWidgetItem *item;
    int cur_index = indexOfItem(root, name);
    if (cur_index >= 0)
    {
        item = root->child(cur_index);
        if (cur_index != index)
        {
            root->takeChild(cur_index);
            root->insertChild(index, item);
        }
        preValue = item->text(1);
    }
    else
    {
        item = new QTreeWidgetItem();        
        item->setText(0,name);
        root->insertChild(index,item);        
    }
    
    item->setText(2, JZNodeType::typeToName(info.type));
    if (JZNodeType::isBaseType(info.type))
        item->setFlags(item->flags() | Qt::ItemIsEditable);

    QString cur_value;
    if(JZNodeType::isBaseType(info.type))
        cur_value = info.value.toString();
    else
    {
        int ptr = info.value.toLongLong();       
        if (info.type == Type_list || info.type == Type_map)
            cur_value = QString("{size = %1}").arg(info.params.size());
        else
            cur_value = "0x" + QString::number(ptr,16);

        QStringList sub_params;
        auto it = info.params.begin();
        int sub_index = 0;
        while (it != info.params.end())
        {        
            setItem(item, sub_index, it.key(), it.value());
            sub_params << it.key();
            sub_index++;
            it++;
        }
        for (int i = item->childCount() - 1; i >= sub_index; i--)
            delete item->takeChild(i);        
    }    
    item->setText(1, cur_value);
    if(!m_newParam && cur_value != preValue)
        item->setTextColor(1, Qt::red);
    else
        item->setTextColor(1, Qt::black);
}

JZNodeDebugParamValue JZNodeWatch::getParamValue(QTreeWidgetItem *item)
{
    JZNodeDebugParamValue value;
    return value;
}

void JZNodeWatch::setParamInfo(JZNodeDebugParamInfo *info, bool isNew)
{   
    m_newParam = isNew;
    if (m_newParam)
    {
        m_view->clear();
    }

    auto root = m_view->invisibleRootItem();    
    QStringList sub_params;
    for (int i = 0; i < info->coors.size(); i++)
    {
        auto &c = info->coors[i];                
        QString name;        
        if (c.type == JZNodeParamCoor::Local || c.type == JZNodeParamCoor::This || c.type == JZNodeParamCoor::Global)
            name = c.name;
        else if (c.type == JZNodeParamCoor::NodeId)
            name = QString::number(c.id);
        else
            name = QString::number(c.id);

        sub_params << name;
        setItem(root, i, name, info->values[i]);
    }
    for (int i = root->childCount() - 1; i >= info->coors.size(); i--)
        delete root->takeChild(i);
    if(m_newParam)
        m_view->invisibleRootItem()->setExpanded(true);
}

void JZNodeWatch::clear()
{
    m_view->clear();    
}