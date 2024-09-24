#include "JZNodeWatch.h"
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
#include "mainwindow.h"

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
    m_mainWindow = nullptr;

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
        clear();
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

void JZNodeWatch::setMainWindow(MainWindow *w)
{
    m_mainWindow = w;
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
            sigParamNameChanged(irRef(item->text(0)));
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
        sigParamValueChanged(coor, item->text(1));
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
        auto p = m_mainWindow->program();
        auto stack = m_mainWindow->stackIndex();
        auto function = m_mainWindow->runtime()->stacks[stack].function;
        auto debug = p->debugInfo(function);
        auto def = debug->nodeParam(param.id());
        if (def)
            return def->name;

        return QString::number(param.id());
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

QTreeWidgetItem *JZNodeWatch::updateItem(QTreeWidgetItem *root, int index, const QString &name, const JZNodeDebugParamValue &info)
{
    QString preValue;
    QTreeWidgetItem *item;
    //设置item位置
    int cur_index = indexOfItem(root, name, index);
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
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(0,name);
        root->insertChild(index,item);        
    }
    
    item->setText(2, editorEnvironment()->typeToName(info.type));
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
            //if (info.type == Type_list || info.type == Type_map)
            //    cur_value = QString("{size = %1}").arg(info.params.size());
        }        
        
        auto it = info.params.begin();
        int sub_index = 0;
        while (it != info.params.end())
        {        
            updateItem(item, sub_index, it.key(), it.value());            
            sub_index++;
            it++;
        }
        for (int i = item->childCount() - 1; i >= sub_index; i--)
            delete item->takeChild(i);        
    }    
    item->setText(1, cur_value);    
    if(cur_value != preValue)
        item->setTextColor(1, Qt::red);
    else
        item->setTextColor(1, Qt::black);

    return item;
}

void JZNodeWatch::setItem(QTreeWidgetItem *root, int index, const JZNodeIRParam &coor, const JZNodeDebugParamValue &info)
{
    auto item = updateItem(root, index, coorName(coor), info);
    if(coor.isStack())
        item->setData(0, Qt::UserRole, coor.id());
}

JZNodeDebugParamValue JZNodeWatch::getParamValue(QTreeWidgetItem *item)
{
    JZNodeDebugParamValue value;
    return value;
}

void JZNodeWatch::setParamInfo(JZNodeGetDebugParamResp *info)
{       
    m_view->blockSignals(true);
    auto root = m_view->invisibleRootItem();        
    for (int i = 0; i < info->coors.size(); i++)                      
        setItem(root, i, info->coors[i], info->values[i]);    
    for (int i = root->childCount() - 1; i >= info->coors.size(); i--)
        delete root->takeChild(i);
    
    updateWatchItem();
    m_view->blockSignals(false);
}

void JZNodeWatch::updateParamInfo(JZNodeGetDebugParamResp *info)
{    
    auto root = m_view->invisibleRootItem();    
    //m_view 中可能存在多个同名的参数，所以此处要通过tree来遍历
    auto count = m_view->topLevelItemCount();
    for (int i = 0; i < count; i++)
    {
        auto item = m_view->topLevelItem(i);        
        for (int j = 0; j < info->coors.size(); j++)
        {
            auto &c = info->coors[j];           
            if (coorName(c) == item->text(0))
            {                
                setItem(root, i, c, info->values[j]);
                break;
            }           
        }
    }    
}

QStringList JZNodeWatch::watchList()
{
    QStringList list;
    auto count = m_view->topLevelItemCount();
    for (int i = 0; i < count; i++)
    {
        QString name = m_view->topLevelItem(i)->text(0);
        if(!name.isEmpty())
            list << name;
    }
    return list;
}

void JZNodeWatch::clear()
{
    m_view->clear();
    updateWatchItem();
}