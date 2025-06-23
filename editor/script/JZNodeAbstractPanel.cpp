#include <QVBoxLayout>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "UiCommon.h"
#include "JZNodeAbstractPanel.h"
#include "JZNodeExpression.h"
#include "JZRegExpHelp.h"
#include "JZNodeFunctionManager.h"
#include "JZProjectItem.h"
#include "JZNodeObject.h"
#include "JZProject.h"
#include "JZScriptItem.h"
#include "JZNodeFunction.h"
#include "JZNodeEvent.h"
#include "JZNodeView.h"
#include "JZNodeFlow.h"
#include "JZNodeValue.h"
#include "JZNodeFactory.h"
#include "JZNodeLocalParamEditDialog.h"
#include "JZNodeEditorManager.h"
#include "JZEditorGlobal.h"

// JZNodeTreeWidget
QMimeData *JZNodeTreeWidget::mimeData(const QList<QTreeWidgetItem *> items) const
{    
    Q_ASSERT(items.size() == 1);
    auto item = items[0];
    if(item->data(0,TreeItem_type).isNull())
        return nullptr;

    QMimeData *mimeData = new QMimeData();
    QString name = item->data(0, TreeItem_type).toString();
    mimeData->setData(name, item->data(0, TreeItem_value).toByteArray());
    return mimeData;
}

//JZNodeAbstractPanel
bool JZNodeAbstractPanel::name_sort(const QString &a, const QString &b)
{
    // 获取两个字符串的首字母，并转换为大写进行比较
    QChar firstA = a[0].toUpper();
    QChar firstB = b[0].toUpper();

    // 如果首字母的大写形式相同，则比较原始首字母（小写在前）
    if (firstA == firstB)
    {
        if (a[0] != b[0])
            return a[0] < b[0]; // 小写字母的 Unicode 值更小
    }

    // 首字母不同时，按大写形式排序
    return a.toLower() < b.toLower();
}

JZNodeAbstractPanel::JZNodeAbstractPanel(QWidget *widget)
    : QWidget(widget)
{    
    m_file = nullptr;
    m_view = nullptr;
    

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);

    m_lineSearch = new QLineEdit();
    layout->addWidget(m_lineSearch);
    connect(m_lineSearch,&QLineEdit::returnPressed,this,&JZNodeAbstractPanel::onSearch);

    m_tree = new JZNodeTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);
    m_tree->setDragEnabled(true);
    m_tree->setDragDropMode(JZNodeTreeWidget::DragOnly);
        
    setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &JZNodeAbstractPanel::onContextMenu);

    layout->addWidget(m_tree);
    setLayout(layout);
}

JZNodeAbstractPanel::~JZNodeAbstractPanel()
{
}

void JZNodeAbstractPanel::setFile(JZScriptItem *file)
{
    m_file = file;    
    m_classFile = m_file->project()->getItemClass(m_file);
    init();
}

void JZNodeAbstractPanel::setView(JZNodeAbstractView *view)
{
    m_view = view;
}

QTreeWidgetItem *JZNodeAbstractPanel::itemOp()
{
    return m_itemOp;
}

QTreeWidgetItem *JZNodeAbstractPanel::itemProcess()
{
    return m_itemProcess;
}

bool JZNodeAbstractPanel::isClassItem(QTreeWidgetItem *item)
{
    auto flag = item->data(0, TreeItem_isClass);
    if (!flag.isValid())
        return false;

    return flag.toBool();
}

QStringList JZNodeAbstractPanel::childItemText(QTreeWidgetItem *root)
{
    QStringList str_list;
    for (int i = 0; i < root->childCount(); i++)
    {
        str_list << root->child(i)->text(0);
    }
    return str_list;
}

void JZNodeAbstractPanel::sortChildItem(QTreeWidgetItem *root)
{
    int count = root->childCount();
    QList<QTreeWidgetItem*> sort_list;
    for (int i = count - 1; i >= 0; i--)
    {
        sort_list << root->takeChild(i);
    }
    std::sort(sort_list.begin(), sort_list.end(), [](const QTreeWidgetItem *a,const QTreeWidgetItem *b)->bool {
        return name_sort(a->text(0),b->text(0));
    });

    for (int i = 0; i < sort_list.size(); i++)
        root->addChild(sort_list[i]);
}

void JZNodeAbstractPanel::syncChildList(QTreeWidgetItem *root, QStringList new_list, int type)
{
    bool is_sort = false;

    //之前有序，删除也有序
    QList<QTreeWidgetItem*> need_remove;
    for (int i = 0; i < root->childCount(); i++)
    {
        QString name = root->child(i)->text(0);
        if (!new_list.contains(name))
            need_remove << root->child(i);
    }
    for (int i = 0; i < need_remove.size(); i++)
        root->removeChild(need_remove[i]);

    //新增，要重新排序
    QStringList cur_list = childItemText(root);
    for (int i = 0; i < new_list.size(); i++)
    {
        if (!cur_list.contains(new_list[i]))
        {
            if(type == Create_Function)
                root->addChild(createFunction(new_list[i]));
            else if (type == Create_Param)
                root->addChild(createParam(new_list[i]));
            else
                root->addChild(createClass(new_list[i]));

            is_sort = true;
        }
    }

    if(is_sort)
        sortChildItem(root);
}

QTreeWidgetItem *JZNodeAbstractPanel::createFolder(QString name)
{
    QString icon_path = ":/JZNodeEditor/Resources/icons/iconFolder.png";

    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setIcon(0, QIcon(icon_path));

    item->setText(0, name);
    item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
    return item;
}

void JZNodeAbstractPanel::setNode(QTreeWidgetItem *item,JZNode *node)
{
    item->setText(0, node->name());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0, TreeItem_type, "node_data");
    item->setData(0,TreeItem_value, editorNodeFactory()->saveNode(node));
}

QTreeWidgetItem *JZNodeAbstractPanel::createNode(JZNode *node)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    setNode(item,node);    
    return item;
}

QTreeWidgetItem *JZNodeAbstractPanel::createParam(QString name)
{
    QStringList names = name.split(".");

    QString full_name = name;
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, names.back());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0,TreeItem_type,"node_param");    
    item->setData(0,TreeItem_value,full_name);

    return item;
}

QTreeWidgetItem *JZNodeAbstractPanel::createFunction(QString name)
{        
    auto coor = JZFunctionHelper::splitFunction(name);

    JZNodeFunction func_node;
    func_node.setFunction(name);

    QTreeWidgetItem *item = createNode(&func_node);
    item->setText(0, coor.name);
    return item;
}

QTreeWidgetItem * JZNodeAbstractPanel::createClass(QString class_name)
{
    QTreeWidgetItem *item_class = new QTreeWidgetItem();
    item_class->setText(0, class_name);
    item_class->setData(0, TreeItem_isClass, true);
    item_class->setData(0, TreeItem_type, "node_class");
    item_class->setData(0, TreeItem_value, class_name);
    return item_class;
}

void JZNodeAbstractPanel::initExpression(QTreeWidgetItem *root, bool flow)
{        
    QTreeWidgetItem *itemExpr = createFolder("运算符");

    for (int i = Node_add; i <= Node_expr; i++)
    {   
        auto node =  dynamic_cast<JZNodeOperator*>(editorNodeFactory()->createNode(i));
        node->setFlow(flow);

        QTreeWidgetItem *sub = createNode(node);
        itemExpr->addChild(sub);
        delete node;
    }
    root->addChild(itemExpr);
}

void JZNodeAbstractPanel::initProcess(QTreeWidgetItem *root)
{
    QTreeWidgetItem *item_process = createFolder("过程");    ;
    m_itemProcess = item_process;

    JZNodeIf node_if;
    JZNodeSwitch node_switch;
    JZNodeSequence node_sequence;
    JZNodeWhile node_while;
    JZNodeFor node_for;
    JZNodeForEach node_foreach;
    JZNodeContinue node_continue;
    JZNodeBreak node_break;    
    JZNodeNop node_nop;
    JZNodeReturn node_return;
    node_return.setFunction(&m_file->function());    
    JZNodeTryCatch node_try;
    JZNodeThrow node_throw;
    
    item_process->addChild(createNode(&node_nop));
    item_process->addChild(createNode(&node_sequence));
    item_process->addChild(createNode(&node_if));
    item_process->addChild(createNode(&node_switch));        
    item_process->addChild(createNode(&node_while));
    item_process->addChild(createNode(&node_for));
    item_process->addChild(createNode(&node_foreach));
    item_process->addChild(createNode(&node_continue));
    item_process->addChild(createNode(&node_break));        
    item_process->addChild(createNode(&node_return));
    item_process->addChild(createNode(&node_try));
    item_process->addChild(createNode(&node_throw));    

    root->addChild(item_process);
}

void JZNodeAbstractPanel::initLocalParam(QTreeWidgetItem *root)
{
    QPushButton *btn = new QPushButton("+");
    btn->setMaximumWidth(24);
    connect(btn, &QPushButton::clicked, this, &JZNodeAbstractPanel::onAddScriptParam);

    QWidget *w = new QWidget();
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(new QLabel("局部变量"));
    l->addStretch();
    l->addWidget(btn);
    w->setLayout(l);

    m_itemLocalParam = createFolder("");
    root->addChild(m_itemLocalParam);
    m_tree->setItemWidget(m_itemLocalParam, 0, w);    
}

void JZNodeAbstractPanel::updateLocalParam()
{
    QStringList local_params = m_file->localVariableList(false);
    syncChildList(m_itemLocalParam, local_params, Create_Param);
}

const JZModule *JZNodeAbstractPanel::module(QString name)
{    
    return JZModuleManager::instance()->module(name);
}

void JZNodeAbstractPanel::removeItem(QTreeWidgetItem *root, QString name)
{
    int index = UiHelper::treeIndexOf(root, name);
    if (index >= 0)
        root->removeChild(root->child(index));
}

bool JZNodeAbstractPanel::filterItem(QTreeWidgetItem *item,QString name)
{
    bool show = false;
    int count = item->childCount();
    bool has_name = item->text(0).contains(name,Qt::CaseInsensitive);

    if(count == 0)
    {
        show = has_name;
    }
    else
    {
        if(isClassItem(item) && has_name)
        {
            show = true;
            for (int i = 0; i < count; i++)
                filterItem(item->child(i), QString());
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                if (filterItem(item->child(i), name))
                    show = true;
            }
        }
    }
    item->setHidden(!show);
    return show;
}

void JZNodeAbstractPanel::onSearch()
{
    QString name = m_lineSearch->text();
    filterItem(m_tree->invisibleRootItem(),name);
    m_tree->expandAll();
}

void JZNodeAbstractPanel::onContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_tree->itemAt(pos);
    if (!item)
        return;

    QMenu menu;
    if (item->parent() != m_itemLocalParam)
        return;
       
    auto actEdit = menu.addAction("编辑");
    auto actDel = menu.addAction("删除");

    auto act = menu.exec(m_tree->mapToGlobal(pos));
    if (!act)
        return;

    if (act == actEdit)
    {
        auto old_def = m_file->localVariable(item->text(0));
        Q_ASSERT(old_def);

        JZNodeLocalParamEditDialog dialog(this);;
        dialog.setParam(*old_def);
        if (dialog.exec() != QDialog::Accepted)
            return;

        auto new_def = dialog.param();
        m_view->changeLocalVariableCommand(old_def->name, new_def);               
    }
    else if (act == actDel)
    {
        m_view->removeLocalVariableCommand(item->text(0));        
    }
}

void JZNodeAbstractPanel::onAddScriptParam()
{
    JZParamDefine define;
    define.name = JZRegExpHelp::uniqueString("localVar", m_file->localVariableList(true));
    define.type = "int";

    JZNodeLocalParamEditDialog dialog(this);
    dialog.setParam(define);
    if (dialog.exec() != QDialog::Accepted)
        return;
    
    define = dialog.param();
    m_view->addLocalVariableCommand(define);       
}

QTreeWidgetItem *JZNodeAbstractPanel::localVariableItem(QString name)
{
    int index = UiHelper::treeIndexOf(m_itemLocalParam, name);
    if (index == -1)
        return nullptr;

    return m_itemLocalParam->child(index);
}