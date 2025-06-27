#include <QTabWidget>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGridLayout>
#include <QPushButton>
#include <QFontMetrics>
#include <QTableWidget>
#include "JZVisionSettingDialog.h"
#include "JZScriptItem.h"
#include "JZVisionView.h"
#include "UiCommon.h"

class VisionLinkVisitor : public JZScriptItemVisitor
{
public:
    VisionLinkVisitor()
    {
        finish = false;
    }

    virtual void visitSelf(JZNode* node)
    {
        if (finish)
            return;

        if (node->isFlowNode())
        {
            if (node == target)
                finish = true;
            else
                nodeList << node;
        }
    }

    QList<JZNode*> nodeList;
    JZNode* target;
    int finish;
};

//JZVisionLinkDialog
JZVisionLinkDialog::JZVisionLinkDialog(QWidget* w)
    :JZBaseDialog(w)
{
    m_view = nullptr;
    m_tree = new QTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);

    m_stacked = new QStackedWidget();
    m_stacked->addWidget(m_tree);

    auto tips = new QLabel();
    tips->setText("没有合适的参数可以连接");
    tips->setAlignment(Qt::AlignCenter);
    m_stacked->addWidget(tips);
    setCentralWidget(m_stacked);
    
    m_node = nullptr;
    m_pinId = -1;
    m_linkId = 0;
}

JZVisionLinkDialog::~JZVisionLinkDialog()
{
}

void JZVisionLinkDialog::setView(JZVisionView* view)
{
    m_view = view;
}

void JZVisionLinkDialog::initLinkList(JZNode *node,int pin_id)
{
    m_node = node;
    m_pinId = pin_id;

    auto env = m_node->environment();
    QList<int> dst_types = env->nameListToTypeList(m_node->pinType(m_pinId));

    auto script = m_node->file();
    JZScriptClassItem *cls_item = script->getClassItem();
    //class
    QTreeWidgetItem *global_item = new QTreeWidgetItem();
    auto member_list = cls_item->memberVariableList(false);
    for(int i = 0; i < member_list.size(); i++)
    {
        auto param_def = cls_item->memberVariable(member_list[i],false);

        JZVisionParamLink member_link;
        member_link.type = JZVisionParamLink::Link_Member;
        member_link.path << param_def->name;

        addLinkItem(global_item, member_link, param_def,dst_types);
    }
    m_tree->addTopLevelItem(global_item);
    if(global_item->childCount() == 0)
        m_tree->setItemHidden(global_item,true);

    //local
    QTreeWidgetItem *local_item = new QTreeWidgetItem();
    auto local_list = script->localVariableList(false);
    for(int i = 0; i < local_list.size(); i++)
    {
        auto param_def = script->localVariable(local_list[i]);

        JZVisionParamLink local_link;
        local_link.type = JZVisionParamLink::Link_Node;
        local_link.path << param_def->name;

        addLinkItem(global_item, local_link, param_def,dst_types);
    }
    m_tree->addTopLevelItem(local_item);
    if(local_item->childCount() == 0)
        m_tree->setItemHidden(local_item, true);
    
    //node
    VisionLinkVisitor visitor;
    visitor.setScript(m_node->file());
    visitor.target = m_node;
    visitor.visit();
    QList<JZNode*> in_list = visitor.nodeList;
    for (int node_idx = 0; node_idx < in_list.size(); node_idx++)
    {
        auto in_node = in_list[node_idx];
        auto out_list = in_node->paramOutList();

        QTreeWidgetItem *cur_node_item = new QTreeWidgetItem();
        cur_node_item->setText(0, m_view->nodeName(in_node->id()));
        for (int i = 0; i < out_list.size(); i++)
        {
            int out_pin = out_list[i];
            QList<int> src_types = env->nameListToTypeList(in_node->pinType(out_pin));
                
            int src_type = env->upType(src_types);
            if (src_type != Type_none)
            {
                JZVisionParamLink node_link;
                node_link.type = JZVisionParamLink::Link_Node;
                node_link.gemo = JZNodeGemo(in_node->id(),out_pin);

                JZParamDefine param_def;
                param_def.name = in_node->pinName(out_pin);
                param_def.type = env->typeToName(src_type);
                addLinkItem(cur_node_item, node_link, &param_def, dst_types);
            }
        }
        if (cur_node_item->childCount() > 0)
            m_tree->addTopLevelItem(cur_node_item);
        else
            delete cur_node_item;
    }    

    if (m_linkId == 0)
        m_stacked->setCurrentIndex(1);
    else
        m_tree->expandAll();
}

void JZVisionLinkDialog::addLinkItem(QTreeWidgetItem *parent,const JZVisionParamLink &link_info,const JZParamDefine* param,const QList<int> &dst_types)
{
    auto env = m_node->environment();
    QList<int> src_types = { env->nameToType(param->type) };

    int dst_type = env->matchType(src_types, dst_types);
    if (dst_type != Type_none)
    {            
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, param->name);
        item->setData(0,Qt::UserRole,m_linkId);
        parent->addChild(item);

        JZVisionParamLink link = link_info;
        if(!link.path.isEmpty())
            link.paramType = env->typeToName(dst_type);

        m_paramLink[m_linkId] = link;
        m_linkId++;
    }
    else
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, param->name);

        if(env->isObject(param->type))
        {
            const JZNodeObjectDefine *obj_def = env->meta(param->type);
            auto param_list = obj_def->paramList(true);
            for(int i = 0; i < param_list.size(); i++)
            {
                const JZParamDefine* sub_param = obj_def->param(param_list[i]);
                JZVisionParamLink sub_link = link_info;
                sub_link.path << sub_param->name;
                addLinkItem(item,sub_link, sub_param, dst_types);
            }
        }

        int vaild_count = 0;
        auto get_vaild = [&vaild_count](QTreeWidgetItem *item){
            if(item->data(0,Qt::UserRole).isValid())
                vaild_count++;
        };

        UiHelper::treeVisit(item,get_vaild);
        if(vaild_count > 0)
            parent->addChild(item);
        else
            delete item;
    }
}

void JZVisionLinkDialog::setLink(JZVisionParamLink link)
{
    int link_id = m_paramLink.key(link);
    auto items = UiHelper::treeFindItem(m_tree->invisibleRootItem(),0,Qt::UserRole,link_id);
    if(items.size() == 1)
        m_tree->setItemSelected(items[0],true);
}

JZVisionParamLink JZVisionLinkDialog::link()
{
    auto selects = m_tree->selectedItems();
    if (selects.size() != 1)
        return JZVisionParamLink();
    
    auto item = selects[0];
    QVariant link_id = item->data(0,Qt::UserRole);
    if(link_id.isValid())
        return m_paramLink[link_id.toInt()];
    else
        return JZVisionParamLink();
}

void JZVisionLinkDialog::accept() 
{    
    auto result = link();
    if(result.isNull())
        return;

    JZBaseDialog::accept();
}

//JZVisionSettingPinWidget
JZVisionSettingPinWidget::JZVisionSettingPinWidget()
{
    m_node = nullptr;
    m_pinEditor = nullptr;    
    m_linkEdit = nullptr;
    m_setting = nullptr;
    m_pinId = -1;

    m_isLink = false;
    m_btnLink = nullptr;    
}

JZVisionSettingPinWidget::~JZVisionSettingPinWidget()
{
}

int JZVisionSettingPinWidget::pinId()
{
    return m_pinId;
}

JZVisionSettingDialog *JZVisionSettingPinWidget::setting()
{
    return m_setting;
}

void JZVisionSettingPinWidget::setSetting(JZVisionSettingDialog *dlg)
{
    m_setting = dlg;
}

void JZVisionSettingPinWidget::setPin(JZNode* node, int pin_id)
{
    m_node = node;
    m_pinId = pin_id;

    auto env = m_node->environment();    
    auto pin = m_node->pin(pin_id);

    QHBoxLayout* l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);    
    this->setLayout(l);

    //link
    m_linkGemo = m_setting->view()->linkInfo(node->id(),pin_id);
    if (m_linkGemo.isNull())
        m_isLink = false;
    else
        m_isLink = true;

    m_btnLink = new QToolButton();
    connect(m_btnLink, &QToolButton::clicked, this, &JZVisionSettingPinWidget::onBtnLink);
    l->addWidget(m_btnLink);    
    
    QString up_type = env->upType(pin->dataType());
    JZParamEditInfo info = JZParamEditInfo::createByPin(m_node, m_pinId);
    if (pin->isConstValue() || pin->flag() & Pin_noCompiler)
    {
        m_btnLink->hide();        
    }
    else if (info.type == JZParamEditInfo::Edit_none)
    {
        m_btnLink->hide();
        m_isLink = true;
    }    
    updatePinWidget();
}

void JZVisionSettingPinWidget::updatePinWidget()
{
    auto env = m_node->environment();

    QHBoxLayout *l = qobject_cast<QHBoxLayout*>(layout());
    if (!m_isLink)
    {
        if (m_linkEdit)
        {
            delete m_linkEdit;
            m_linkEdit = nullptr;
        }
        if (!m_pinEditor)
        {
            JZParamEditInfo info = JZParamEditInfo::createByPin(m_node, m_pinId);
            m_pinEditor = new JZNodeParamValueWidget();
            m_pinEditor->init(info);            
            m_pinEditor->setValue(m_node->pinValue(m_pinId));

            QHBoxLayout *h_l = new QHBoxLayout();            
            l->insertWidget(1, m_pinEditor);
        }
        m_btnLink->setText("连接输入");
    }
    else
    {
        if (m_pinEditor)
        {
            delete m_pinEditor;
            m_pinEditor = nullptr;
        }
        if (!m_linkEdit)
        {                        
            m_linkEdit = new JZLineEditButton();
            l->insertWidget(1, m_linkEdit);
            connect(m_linkEdit->button(),&QToolButton::clicked,this, &JZVisionSettingPinWidget::onLickSelected);
            m_linkEdit->lineEdit()->setText(linkName());
            m_linkEdit->lineEdit()->setReadOnly(true);
        }
        m_btnLink->setText("直接输入");
    }
}

void JZVisionSettingPinWidget::onBtnLink()
{
    m_isLink = !m_isLink;
    updatePinWidget();
}


QString JZVisionSettingPinWidget::linkName()
{
    auto view = m_setting->view();
    if (m_linkGemo.type == JZVisionParamLink::Link_Node)
    {
        QString pin_name = view->pinName(m_linkGemo.gemo);
        if(!m_linkGemo.path.isEmpty())
            pin_name += "." + m_linkGemo.path.join(".");
        
        return pin_name;
    }
    else
        return m_linkGemo.path.join(".");
}

void JZVisionSettingPinWidget::onLickSelected()
{
    JZVisionLinkDialog dlg(this);
    dlg.setView(m_setting->view());
    dlg.initLinkList(m_node, m_pinId);
    dlg.setLink(m_linkGemo);
    if(dlg.exec() != QDialog::Accepted)
        return;

    m_linkGemo = dlg.link();
}

bool JZVisionSettingPinWidget::isLink()
{
    return m_isLink;
}

JZVisionParamLink JZVisionSettingPinWidget::linkInfo()
{
    return m_linkGemo;
}

QString JZVisionSettingPinWidget::value()
{
    return m_pinEditor->value();
}

//JZVisionSettingDialog
JZVisionSettingDialog::JZVisionSettingDialog(QWidget *parent) 
    : JZBaseDialog(parent)
{    
    m_node = nullptr;
    m_layout = nullptr;
}

JZVisionSettingDialog::~JZVisionSettingDialog()
{
}

JZVisionView* JZVisionSettingDialog::view()
{
    return qobject_cast<JZVisionView*>(parentWidget());
}

QMap<int, JZVisionSettingDialog::Block> JZVisionSettingDialog::blockList()
{
    return m_blockList;
}


void JZVisionSettingDialog::onPinAdd()
{
    int id = 0;
    int widget_index = 0;
    if (m_node->type() == Node_if)
    {
        auto node_if = dynamic_cast<JZNodeIf*>(m_node);
        id = node_if->addCondPin();
        widget_index = node_if->condCount();
    }
    else
    {
        auto node_switch = dynamic_cast<JZNodeSwitch*>(m_node);
        id = node_switch->addCase();
        widget_index = node_switch->caseCount();
    }

    JZVisionSettingPinWidget* pin_widget = createPin(m_node->pin(id));
    m_layout->insertWidget(widget_index, pin_widget);

    Block block;
    block.pinId = id;
    block.pinWidget = pin_widget;
    m_blockList.insert(id,block);
}

void JZVisionSettingDialog::onPinRemove()
{
    int id = sender()->property("PinId").toInt();
    if (m_node->type() == Node_if)
    {
        auto node_if = dynamic_cast<JZNodeIf*>(m_node);
        if (id != node_if->elsePin())
        {
            if (node_if->condCount() == 1)
            {
                QMessageBox::information(this, "", "至少保留一个条件");
                return;
            }
            node_if->removeCond(id);
        }
        else
            node_if->removeElse();
    }
    else
    {
        auto node_switch = dynamic_cast<JZNodeSwitch*>(m_node);
        if (id != node_switch->defaultPin())
        {
            if (node_switch->caseCount() == 1)
            {
                QMessageBox::information(this, "", "至少保留一个条件");
                return;
            }
            node_switch->removeCase(id);
        }
        else
            node_switch->removeDefault();
    }

    m_blockList[id].pinWidget->deleteLater();
    m_blockList.remove(id);
}

void JZVisionSettingDialog::onPinElse()
{
    int id = -1;
    if (m_node->type() == Node_if)
    {
        auto node_if = dynamic_cast<JZNodeIf*>(m_node);
        if (!node_if->hasElse())
            id = node_if->addElsePin();
    }
    else
    {
        auto node_switch = dynamic_cast<JZNodeSwitch*>(m_node);
        if (!node_switch->hasDefault())
            id = node_switch->addDefault();
    }
    if (id == -1)
        return;

    int widget_index = m_node->subFlowCount();
    JZVisionSettingPinWidget* pin_widget = createPin(m_node->pin(id));
    m_layout->insertWidget(widget_index, pin_widget);

    Block block;
    block.pinId = id;
    block.pinWidget = pin_widget;
    m_blockList.insert(id, block);
}

void JZVisionSettingDialog::initNodeIfSwitch()
{
    m_layout->addWidget(new QLabel("条件"));
    
    QList<int> pin_list;
    if (m_node->type() == Node_if)
        pin_list = m_node->paramInList();
    else
        pin_list = m_node->paramOutList();

    for (int i = 0; i < pin_list.size(); i++)
    {
        QLabel* item_name = new QLabel(m_node->pinName(pin_list[i]));
        JZVisionSettingPinWidget* pin_widget = createPin(m_node->pin(pin_list[i]));
        QToolButton* btn_remove = new QToolButton();
        btn_remove->setText("-");
        btn_remove->setProperty("PinId", pin_list[i]);
        connect(btn_remove, &QToolButton::clicked, this, &JZVisionSettingDialog::onPinRemove);

        QWidget* w = UiHelper::createHBox({ item_name,pin_widget,btn_remove });
        m_layout->addWidget(w);

        Block block;
        block.pinWidget = pin_widget;
        block.pinId = pin_list[i];
        m_blockList.insert(block.pinId, block);
    }
    
    QPushButton* pin_add = new QPushButton();
    QPushButton* pin_else = new QPushButton();
    connect(pin_add, &QPushButton::clicked, this, &JZVisionSettingDialog::onPinAdd);
    connect(pin_else, &QPushButton::clicked, this, &JZVisionSettingDialog::onPinElse);

    pin_add->setText("增加条件");
    if (m_node->type() == Node_if)
        pin_else->setText("增加else");
    else
        pin_else->setText("增加default");

    QHBoxLayout* pin_l = new QHBoxLayout();
    pin_l->setContentsMargins(0, 0, 0, 0);
    pin_l->addWidget(pin_add);
    pin_l->addWidget(pin_else);
    pin_l->addStretch();
    m_layout->addLayout(pin_l);
}


void JZVisionSettingDialog::initNodeNormal()
{
    auto in_list = m_node->paramInList();
    if (in_list.size() > 0)
    {
        m_layout->addWidget(new QLabel("输入参数"));

        QGridLayout* grid_in = new QGridLayout();
        grid_in->addWidget(new QLabel("名称"), 0, 0);
        grid_in->addWidget(new QLabel("类型"), 0, 1);
        grid_in->addWidget(new QLabel("值"), 0, 2);

        for (int i = 0; i < in_list.size(); i++)
        {
            auto pin = m_node->pin(in_list[i]);

            QLabel* item_name = new QLabel(m_node->pinName(in_list[i]));
            QLabel* item_type = new QLabel(m_node->pinType(in_list[i]).join(", "));

            int row = i + 1;
            grid_in->addWidget(item_name, row, 0);
            grid_in->addWidget(item_type, row, 1);

            JZVisionSettingPinWidget* pin_widget = createPin(m_node->pin(in_list[i]));
            grid_in->addWidget(pin_widget,row, 2);

            Block block;
            block.pinWidget = pin_widget;
            block.pinId = in_list[i];
            m_blockList.insert(block.pinId, block);
        }
        m_layout->addLayout(grid_in);
    }
    auto out_list = m_node->paramOutList();
    if (out_list.size() > 0)
    {
        m_layout->addWidget(new QLabel("输出参数"));

        QGridLayout* grid_out = new QGridLayout();
        grid_out->addWidget(new QLabel("名称"), 0, 0);
        grid_out->addWidget(new QLabel("类型"), 0, 1);
        for (int i = 0; i < out_list.size(); i++)
        {
            QLabel* item_name = new QLabel(m_node->pinName(out_list[i]));
            QLabel* item_type = new QLabel(m_node->pinType(out_list[i]).join(", "));

            int row = i + 1;
            grid_out->addWidget(item_name, row, 0);
            grid_out->addWidget(item_type, row, 1);
        }
        m_layout->addLayout(grid_out);
    }
}

void JZVisionSettingDialog::setNode(JZNode* node)
{
    m_node = node;
    QWidget *area_widget = new QWidget();    
    
    m_layout = new QVBoxLayout();
    area_widget->setLayout(m_layout);

    QLabel *label_name = new QLabel(view()->nodeName(node->id()));
    m_layout->addWidget(label_name);

    //参数    
    if (m_node->type() == Node_if || m_node->type() == Node_switch)
        initNodeIfSwitch();
    else
        initNodeNormal();
    
    m_layout->addStretch();


    QScrollArea *area = new QScrollArea();
    area->setWidgetResizable(true);
    area->setWidget(area_widget);

    setWindowTitle(view()->nodeName(m_node->id()));
    setCentralWidget(area);

    updatePinWidget();
    resize(480, 600);
}

void JZVisionSettingDialog::updatePinWidget()
{
    int name_size = 0;
    auto it = m_blockList.begin();
    while (it != m_blockList.end())
    {
        QString name = m_node->pinName(it->pinId);

        QFontMetrics ft(font());
        name_size = qMax(name_size, ft.horizontalAdvance(name));

        it++;
    }
}

JZVisionSettingPinWidget* JZVisionSettingDialog::createPin(JZNodePin *pin)
{
    JZVisionSettingPinWidget* pin_widget = new JZVisionSettingPinWidget();
    pin_widget->setSetting(this);
    pin_widget->setPin(m_node, pin->id());

    return pin_widget;
}

void JZVisionSettingDialog::accept()
{
    auto it = m_blockList.begin();
    while (it != m_blockList.end())
    {
        auto pin_widget = it->pinWidget;
        if (!pin_widget->isLink())
        {
            m_node->setPinValue(it->pinId, pin_widget->value());
        }

        it++;
    }

    JZBaseDialog::accept();
}