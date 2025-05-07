#include <QGroupBox>
#include <QPainter>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipBoard>
#include <QKeyEvent>
#include <QStyledItemDelegate>
#include "UiCommon.h"
#include "JZNodeEngine.h"
#include "JZNodeWatch.h"
#include "JZEditorGlobal.h"

enum {
    Data_coor = Qt::UserRole
};

class GridDelegate : public QStyledItemDelegate
{
public:
    explicit GridDelegate(QObject * parent = nullptr) : QStyledItemDelegate(parent) { }

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        painter->setPen(QColor(Qt::black)); // set ink to black
        QRect rc = option.rect;
        if (index.column() == 0)
            rc.setX(0);
        painter->drawRect(rc);  // draw a rect covering cell area     
        QStyledItemDelegate::paint(painter, option, index);  // tell it to draw as normally after
    }
};

JZNodeWatch::JZNodeWatch(QWidget *parent)
    :QWidget(parent)
{
    m_status = Process_none;
    m_readOnly = false;
    m_editColumn = 0;
    m_editItem = nullptr;    
    m_nodeItem = nullptr;

    m_view = new QTreeWidget();
    m_view->setColumnCount(3);
    m_view->setHeaderLabels({ "名称","值","类型" });
    m_view->setEditTriggers(QTreeWidget::NoEditTriggers);
    
    GridDelegate *grid = new GridDelegate(m_view);
    m_view->setItemDelegate(grid);

    connect(m_view, &QTreeWidget::itemDoubleClicked, this, &JZNodeWatch::onTreeWidgetItemDoubleClicked);
    connect(m_view, &QTreeWidget::itemChanged, this, &JZNodeWatch::onItemChanged);

    QVBoxLayout *sub_layout = new QVBoxLayout();
    sub_layout->addWidget(m_view);
    this->setLayout(sub_layout);

    updateWatchItem();
}

JZNodeWatch::~JZNodeWatch()
{

}

void JZNodeWatch::updateStatus()
{
    if (m_status == Process_none)
    {
        this->setEnabled(true);     
        setNodeInfo(NodeInfo());
    }
    else if (m_status == Process_running)
        this->setEnabled(false);
    else
        this->setEnabled(true);
}

void JZNodeWatch::updateWatchItem()
{
    if (m_readOnly)
    {
        if (m_view->topLevelItemCount() > 0
            && m_view->topLevelItem(m_view->topLevelItemCount() - 1)->text(0).isEmpty())
            delete m_view->takeTopLevelItem(m_view->topLevelItemCount() - 1);
    }
    else
    {
        if (m_view->topLevelItemCount() == 0
            || !m_view->topLevelItem(m_view->topLevelItemCount() - 1)->text(0).isEmpty())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            m_view->addTopLevelItem(item);
        }        
    }
}

void JZNodeWatch::setReadOnly(bool flag)
{
    m_readOnly = flag;
    updateWatchItem();
}

void JZNodeWatch::setRunningMode(ProcessStatus status)
{
    m_status = status;
    updateStatus();
}

void JZNodeWatch::onTreeWidgetItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    if (m_status != Process_pause)
        return;

    QString name = item->text(0);
    int row = m_view->indexOfTopLevelItem(item);
    if ((!m_readOnly && (column == 0 && row >= 0))
        || (name != "this" && !name.isEmpty() && column == 1))
    {        
        m_editColumn = column;
        m_editItem = item;
        m_view->editItem(item, column);
    }
}

void JZNodeWatch::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (m_editItem != item)
        return;
    m_editItem = nullptr;

    if (m_editColumn == 0)
    {
        if (item->text(0).isEmpty())
        {
            auto row = m_view->indexOfTopLevelItem(item);
            delete m_view->takeTopLevelItem(row);
        }
        else
        {            
            sigGetWatch(irRef(item->text(0)));
        }
    }
    else
    {
        auto vid = item->data(0, Qt::UserRole);

        JZNodeIRParam coor;
        if (vid.isValid())
        {
            coor = irId(vid.toInt());
        }
        else
        {
            coor = irRef(item->text(0));
        }        
        sigSetWatch(coor, item->text(1));
    }    
    updateWatchItem();
}

void JZNodeWatch::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Delete && !m_readOnly) 
    {
        auto item = m_view->currentItem();
        if (!item)
            return;

        int row = m_view->indexOfTopLevelItem(item);
        if (row >= 0 && row != m_view->topLevelItemCount() - 1)
            delete m_view->takeTopLevelItem(row);
    }
}

QString JZNodeWatch::coorName(const JZNodeIRParam &param)
{
    if (param.isThis())
        return "this";
    if (param.isRef())
        return param.ref();
    else
    {
        JZNodeGemo gemo = JZNodeGemo::fromParamId(param.id());
        auto pin = m_nodeInfo.param(gemo.pinId);
        return pin->define.name;
    }
}

int JZNodeWatch::indexOfItem(QTreeWidgetItem *root, const QString &name,int start)
{
    for (int i = start; i < root->childCount(); i++)
    {
        auto sub = root->child(i);
        if (sub->text(0) == name)
            return i;
    }
    return -1;
}

void JZNodeWatch::setItem(QTreeWidgetItem *item, const JZNodeDebugParamValue &info)
{
    auto env = editorEnvironment();

    QString preValue = item->text(1);
    item->setText(2, env->typeToName(info.type));
    if (JZNodeType::isBase(info.type))
        item->setFlags(item->flags() | Qt::ItemIsEditable);

    QString cur_value;
    if (info.type == Type_none)
        cur_value = "未定义";
    else if(JZNodeType::isBaseOrEnum(info.type))
        cur_value = info.value;    
    else
    {       
        cur_value = info.value;
        if (info.value != "null")
        {            
            if (env->isListType(info.type)|| env->isMapType(info.type))
                cur_value = QString("{size = %1}").arg(info.subParamValues.size());
        }        
                
        QList<QTreeWidgetItem *> vaild_sub_items;
        for(int sub_index = 0; sub_index < info.subParamValues.size(); sub_index++)
        {        
            QString sub_name = info.subParamNames[sub_index];
            auto &param = info.subParamValues[sub_index];
            int tree_sub_idx = UiHelper::treeIndexOf(item, sub_name);
            QTreeWidgetItem *sub_item = nullptr;
            if (tree_sub_idx = -1)
            {
                sub_item = new QTreeWidgetItem();
                sub_item->setText(0, sub_name);
            }
            else
            {
                sub_item = item->child(tree_sub_idx);
            }
            setItem(sub_item, param);
            vaild_sub_items << sub_item;
        }
        for (int i = item->childCount() - 1; i >= 0; i--)
        {
            if(vaild_sub_items.contains(item->child(i)))
                delete item->takeChild(i);
        }
        UiHelper::treeSortChilds(item);        
    }            
    if (cur_value != preValue)
    {
        item->setText(1, cur_value);
        item->setTextColor(1, Qt::red);
    }
    else
        item->setTextColor(1, Qt::black);
}

void JZNodeWatch::setNodeInfo(const NodeInfo &info)
{
    m_view->blockSignals(true);
    if (m_nodeInfo.id != info.id)
    {
        m_nodeInfo = info;
        
        if (m_nodeItem)
        {
            int index = m_view->indexOfTopLevelItem(m_nodeItem);
            delete m_view->takeTopLevelItem(index);
            m_nodeItem = nullptr;
        }

        if (m_nodeInfo.id != INVALID_ID)
        {
            m_nodeItem = new QTreeWidgetItem();
            m_nodeItem->setText(0, info.name);
            m_view->insertTopLevelItem(0, m_nodeItem);

            for (int i = 0; i < info.params.size(); i++)
            {
                QTreeWidgetItem *sub = new QTreeWidgetItem();
                sub->setText(0, info.params[i].define.name);
                m_nodeItem->addChild(sub);
            }
            m_nodeItem->setExpanded(true);
        }
    }
    m_view->blockSignals(false);
}

void JZNodeWatch::updateParamInfo(JZNodeGetDebugParamResp *info)
{
    m_view->blockSignals(true);    
    
    auto findAndSet = [this,info](QTreeWidgetItem *item,bool isStack) 
    {        
        for (int j = 0; j < info->req.coors.size(); j++)
        {
            auto &c = info->req.coors[j];
            if (coorName(c) == item->text(0) && info->req.coors[j].isStack() == isStack)
            {
                setItem(item, info->values[j]);
                return;
            }
        }
        UiHelper::treeClearChildren(item);
        item->setText(1, "no such item");
    };

    int start = 0;
    if (m_nodeItem)
    {
        for (int i = 0; i < m_nodeItem->childCount(); i++)
        {
            auto item = m_nodeItem->child(i);
            findAndSet(item, true);
        }
        start = 1;
    }
    
    for (int i = start; i < m_view->topLevelItemCount() - 1; i++) //最后一个是新建变量
    {
        auto item = m_view->topLevelItem(i);
        findAndSet(item, false);
    }

    m_view->blockSignals(false);
}

QStringList JZNodeWatch::watchList()
{
    QStringList list;
    auto count = m_view->topLevelItemCount();
    for (int i = 0; i < count; i++)
    {
        if (m_view->topLevelItem(i) == m_nodeItem)
            continue;

        QString name = m_view->topLevelItem(i)->text(0);
        if(!name.isEmpty())
            list << name;
    }
    return list;
}