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
        if (statusIsPause(Status_pause))
            this->setEnabled(true);
        else
            this->setEnabled(false);
    }
    else
    {
        this->setEnabled(true);
        m_view->clear();
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

QTreeWidgetItem *JZNodeWatch::createItem(QString name, const JZNodeDebugParamValue &info)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0,name);
    item->setText(2, JZNodeType::typeToName(info.type));
    if (JZNodeType::isBaseType(info.type))
        item->setFlags(item->flags() | Qt::ItemIsEditable);

    if(JZNodeType::isBaseType(info.type))
        item->setText(1, info.value.toString());
    else
    {
        int ptr = info.value.toLongLong();        
        item->setText(1, "0x" + QString::number(ptr,16));
        auto it = info.params.begin();
        while (it != info.params.end())
        {        
            QTreeWidgetItem *sub = createItem(it.key(), it.value());
            item->addChild(sub);
            it++;
        }
    }
    return item;
}

JZNodeDebugParamValue JZNodeWatch::getParamValue(QTreeWidgetItem *item)
{
    JZNodeDebugParamValue value;
    return value;
}

void JZNodeWatch::setParamInfo(JZNodeDebugParamInfo *info)
{
    m_view->clear();
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

        QTreeWidgetItem *item = createItem(name, info->values[i]);
        m_view->addTopLevelItem(item);
    }
    m_view->invisibleRootItem()->setExpanded(true);
}

void JZNodeWatch::clear()
{
    m_view->clear();
}