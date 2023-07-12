#include "JZNodePanel.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QLineEdit>
#include "JZNodeFunctionManager.h"
#include "JZNodeValue.h"
#include "JZNodeFactory.h"
#include "JZProjectItem.h"
#include "JZNodeObject.h"
#include "JZProject.h"
#include "JZScriptFile.h"

enum{
    TreeItem_type = Qt::UserRole,
    TreeItem_value,
};

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

// JZNodePanel
JZNodePanel::JZNodePanel(QWidget *widget)
    : QWidget(widget)
{
    m_fileType = 0;
    m_file = nullptr;
    m_classFile = nullptr;

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);

    m_lineSearch = new QLineEdit();
    layout->addWidget(m_lineSearch);
    connect(m_lineSearch,&QLineEdit::returnPressed,this,&JZNodePanel::onSearch);

    m_tree = new JZNodeTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);
    m_tree->setDragEnabled(true);   

    layout->addWidget(m_tree);
    setLayout(layout);
}

JZNodePanel::~JZNodePanel()
{
}

void JZNodePanel::setFile(JZScriptFile *file)
{
    m_file = file;
    m_fileType = m_file->itemType();
    m_classFile = m_file->getClassFile();
    init();
}

void JZNodePanel::init()
{
    m_tree->clear();


    QTreeWidgetItem *itemParam = createFolder("全局变量");
    m_tree->addTopLevelItem(itemParam);
    initProjectParam(itemParam);

    if(m_classFile){
        QTreeWidgetItem *itemClassParam = createFolder("成员变量");
        m_tree->addTopLevelItem(itemClassParam);
        initClassParam(itemClassParam);
    }

    QTreeWidgetItem *itemData = createFolder("数据");
    m_tree->addTopLevelItem(itemData);
    initVariable(itemData);

    QTreeWidgetItem *itemExpr = createFolder("计算");
    m_tree->addTopLevelItem(itemExpr);
    initExpression(itemExpr);

    if(m_fileType != ProjectItem_scriptParamBinding)
    {
        QTreeWidgetItem *itemProcess = createFolder("类");
        m_tree->addTopLevelItem(itemProcess);
        initProcess(itemProcess);
    }
    m_tree->expandAll();
}

QTreeWidgetItem *JZNodePanel::createFolder(QString name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);
    item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
    return item;
}

QTreeWidgetItem *JZNodePanel::createNode(JZNode *node)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, node->name());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0, TreeItem_type, "node_data");
    item->setData(0, TreeItem_value, formatNode(node));
    return item;
}

QTreeWidgetItem *JZNodePanel::createParam(QString name,int dataType,QString preName)
{
    QString full_name = name;
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0,TreeItem_type,"node_param");
    if(!preName.isEmpty())
        full_name = preName + "." + name;
    item->setData(0,TreeItem_value,full_name);

    if(JZNodeType::isObject(dataType))
    {
        auto def = JZNodeObjectManager::instance()->meta(dataType);
        auto list = def->paramList();
        for(int i = 0; i < list.size(); i++)
        {
            auto sub_def = def->param(list[i]);
            QTreeWidgetItem *sub_item = createParam(list[i],sub_def->dataType,full_name);
            item->addChild(sub_item);
        }
    }

    return item;
}

void JZNodePanel::initClassParam(QTreeWidgetItem *root)
{    
    auto def = m_classFile->objectDefine();
    auto list = def.paramList();
    for(int i = 0; i < list.size(); i++)
    {
        auto info = &def.params[list[i]];
        QTreeWidgetItem *item = createParam(list[i],info->dataType,"this");
        root->addChild(item);
    }
}

void JZNodePanel::initProjectParam(QTreeWidgetItem *root)
{
    auto project = m_file->project();
    auto list = project->variableList();
    for(int i = 0; i < list.size(); i++)
    {
        auto info = project->getVariableInfo(list[i]);
        QTreeWidgetItem *item = createParam(list[i],info->dataType,"");
        root->addChild(item);
    }
}

void JZNodePanel::initEvent(QTreeWidgetItem *root)
{    
    QTreeWidgetItem *item_event = createFolder("事件");

    JZNodeParamChangedEvent event;
    item_event->addChild(createNode(&event));

    root->addChild(item_event);
}

void JZNodePanel::initVariable(QTreeWidgetItem *root)
{
    JZNodeLiteral node_int,node_bool,node_int64,node_string,node_double;
    node_int.setName("整数");
    node_int.setLiteral(0);
    node_int.setDataType(Type_int);

    node_bool.setName("Bool");
    node_bool.setLiteral(false);
    node_bool.setDataType(Type_bool);

    node_string.setName("字符串");
    node_string.setLiteral("");
    node_string.setDataType(Type_string);

    node_double.setName("浮点数");
    node_double.setLiteral(0.0);
    node_double.setDataType(Type_double);

    root->addChild(createNode(&node_int));    
    root->addChild(createNode(&node_string));
    root->addChild(createNode(&node_double));
    root->addChild(createNode(&node_bool));

    JZNodeParam get;
    JZNodeSetParam set;
    JZNodeSetParamDataFlow set_data;
    JZNodeCreate create;
    root->addChild(createNode(&get));
    if(m_fileType == ProjectItem_scriptParamBinding)
    {
        root->addChild(createNode(&set_data));
    }
    else
    {
        root->addChild(createNode(&set));
        root->addChild(createNode(&create));
    }

    if(m_classFile)
    {
        JZNodeThis node_this;
        root->addChild(createNode(&node_this));
    }
}


void JZNodePanel::initExpression(QTreeWidgetItem *root)
{    
    QTreeWidgetItem *item_op = createFolder("算子");

    for (int i = Node_add; i <= Node_expr; i++)
    {   
        auto node = JZNodeFactory::instance()->createNode(i);
        QTreeWidgetItem *sub = createNode(node);
        item_op->addChild(sub);
        delete node;
    }

    QTreeWidgetItem *item_func = createFolder("函数");
    initFunction(item_func,false);

    root->addChild(item_op);
    root->addChild(item_func);
}

void JZNodePanel::initFunction(QTreeWidgetItem *root,bool flow)
{
    QMap<QString,QTreeWidgetItem*> itemMap;

    auto funcs = JZNodeFunctionManager::instance()->functionList();
    for (int i = 0; i < funcs.size(); i++)
    {
        if(funcs[i]->isFlowFunction != flow)
            continue;

        QString function = funcs[i]->name;

        int index = function.indexOf(".");
        QString class_name,short_name;
        if(index != -1)
        {
            class_name = function.left(index);
            short_name = function.mid(index + 1);
        }
        else
        {
            class_name = "全局";
            short_name = function;
        }

        QTreeWidgetItem *item_class;
        if(!itemMap.contains(class_name))
        {
            item_class = new QTreeWidgetItem();
            item_class->setText(0,class_name);
            itemMap[class_name] = item_class;
            root->addChild(item_class);
        }
        else
        {
            item_class = itemMap[class_name];
        }

        JZNodeFunction node_func;
        node_func.setName(function);
        node_func.setFunction(funcs[i]);

        auto function_node = createNode(&node_func);
        function_node->setText(0, short_name);
        item_class->addChild(function_node);
    }
}

void JZNodePanel::initProcess(QTreeWidgetItem *root)
{
    QTreeWidgetItem *item_process = new QTreeWidgetItem();
    item_process->setText(0,"基本过程");    

    JZNodeFor node_for;
    JZNodeWhile node_while;
    JZNodeSequence node_seq;
    JZNodeBranch node_branch;
    JZNodeForEach node_foreach;
    JZNodeBreak node_break;
    JZNodeContinue node_continue;
    JZNodeExit node_exit;
    item_process->addChild(createNode(&node_seq));
    item_process->addChild(createNode(&node_branch));
    item_process->addChild(createNode(&node_while));
    item_process->addChild(createNode(&node_for));
    item_process->addChild(createNode(&node_foreach));
    item_process->addChild(createNode(&node_break));
    item_process->addChild(createNode(&node_continue));
    item_process->addChild(createNode(&node_exit));

    QTreeWidgetItem *item_class = createFolder("类");
    item_class->addChild(createClass("list"));
    item_class->addChild(createClass("widget"));
    item_class->addChild(createClass("LineEdit"));
    item_class->addChild(createClass("PushButton"));

    root->addChild(item_process);
    initEvent(root);
    root->addChild(item_class);
}

QTreeWidgetItem *JZNodePanel::createClass(QString className)
{
    QTreeWidgetItem *item_class = new QTreeWidgetItem();
    item_class->setText(0,className);

    QTreeWidgetItem *item_func = nullptr;
    QTreeWidgetItem *item_single = nullptr;

    auto meta = JZNodeObjectManager::instance()->meta(className);
    Q_ASSERT(meta);
    auto funcs = JZNodeFunctionManager::instance()->functionList();
    for (int i = 0; i < funcs.size(); i++)
    {
        int index = -1;
        if((index = funcs[i]->name.indexOf(".")) == -1)
            continue;

        QString func_class = funcs[i]->name.left(index);
        QString short_name = funcs[i]->name.mid(index + 1);
        if(meta->isInherits(func_class))
        {
            if(meta->single(short_name))
            {
                JZNodeSingleEvent node_event;
                node_event.setName(funcs[i]->name);
                node_event.setSingle(func_class,meta->single(short_name));

                if(item_single == nullptr)
                {
                    item_single = new QTreeWidgetItem();
                    item_single->setText(0, "事件");
                }

                QTreeWidgetItem *sub_event = new QTreeWidgetItem();
                sub_event->setData(0, TreeItem_type, "node_data");
                sub_event->setText(0, short_name);
                sub_event->setData(0, TreeItem_value, formatNode(&node_event));
                item_single->addChild(sub_event);
            }

            QTreeWidgetItem *sub_function = new QTreeWidgetItem();
            sub_function->setData(0, TreeItem_type, "node_data");
            sub_function->setText(0, short_name);

            JZNodeFunction node_func;
            node_func.setName(funcs[i]->name);
            node_func.setFunction(funcs[i]);

            if(item_func == nullptr)
            {
                item_func = new QTreeWidgetItem();
                item_func->setText(0, "function");
            }
            sub_function->setData(0, TreeItem_value, formatNode(&node_func));
            item_func->addChild(sub_function);
        }
    }

    if(item_single)
        item_class->addChild(item_single);
    if(item_func)
        item_class->addChild(item_func);

    return item_class;
}

bool JZNodePanel::filterItem(QTreeWidgetItem *item,QString name)
{
    bool show = false;
    int count = item->childCount();
    if(count == 0)
    {
        show = item->text(0).contains(name);
    }
    else
    {
        for(int i = 0; i < count; i++)
        {
            if(filterItem(item->child(i),name))
                show = true;
        }
    }
    item->setHidden(!show);
    return show;
}

void JZNodePanel::onSearch()
{
    QString name = m_lineSearch->text();
    filterItem(m_tree->invisibleRootItem(),name);
}
