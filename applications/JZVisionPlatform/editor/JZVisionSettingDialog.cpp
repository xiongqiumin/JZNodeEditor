#include <QTabWidget>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGridLayout>
#include "JZVisionSettingDialog.h"
#include "JZScriptItem.h"
#include "JZVisionView.h"
#include "UiCommon.h"


//JZVisionLinkDialog
JZVisionLinkDialog::JZVisionLinkDialog(QWidget* w)
    :JZBaseDialog(w)
{
    m_tree = new QTreeWidget();
    setCentralWidget(m_tree);
    
    m_node = nullptr;
    m_pinId = -1;
    m_linkId = 0;
}

void JZVisionLinkDialog::initLinkList(JZNode *node,int pin_id)
{
    m_node = node;
    m_pinId = -1;

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
    auto local_list = script->localVariableList(true);
    for(int i = 0; i < member_list.size(); i++)
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

    QTreeWidgetItem *node_item = new QTreeWidgetItem();

    //node
    JZScriptItemVisitor visitor(m_node->file());
    QList<JZNode*> in_list = visitor.flowInputNodeRecursively(m_node);
    for (int node_idx = 0; node_idx < in_list.size(); node_idx++)
    {
        auto in_node = in_list[node_idx];
        auto out_list = in_node->paramOutList();

        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, in_node->name());
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
                param_def.name = node->pinName(out_pin);
                param_def.type = env->typeToName(src_type);
                addLinkItem(node_item, node_link, &param_def, dst_types);
            }
        }
    }

    if(node_item->childCount() == 0)
        m_tree->setItemHidden(node_item,true);

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
    bool ok = false;
    int link_id = m_tree->currentItem()->data(0,Qt::UserRole).toInt(&ok);
    if(ok)
        return m_paramLink[link_id];
    else
        return JZVisionParamLink();
}

void JZVisionLinkDialog::accept() 
{
    auto result = link();
    if(result.gemo.isNull())
        return;

    accept();
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
    connect(this, &JZVisionSettingPinWidget::sigPinRemove, m_setting, &JZVisionSettingDialog::onPinRemove);
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

    //name
    QLabel *label = new QLabel(pin->name() + ": ");
    l->addWidget(label);

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
    JZParamEditInfo info = JZParamEditInfo::createType(env, up_type);
    if (pin->isConstValue())
    {
        m_btnLink->hide();        
    }
    else if (info.type == JZParamEditInfo::Edit_none)
    {
        m_btnLink->hide();
        m_isLink = true;
    }    

    if (m_node->type() == Node_if || m_node->type() == Node_switch)
    {
        QToolButton* pin_remove = new QToolButton();
        pin_remove->setText("-");
        connect(pin_remove, &QToolButton::clicked, this, &JZVisionSettingPinWidget::sigPinRemove);
        l->addWidget(pin_remove);
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
            auto pin = m_node->pin(m_pinId);
            QString up_type = env->upType(pin->dataType());
            JZParamEditInfo info = JZParamEditInfo::createType(env, up_type);
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
        return view->pinName(m_linkGemo.gemo) + "." + m_linkGemo.path.join(".");
    else
        return m_linkGemo.path.join(".");
}

void JZVisionSettingPinWidget::onLickSelected()
{
    JZVisionLinkDialog dlg(this);
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
    m_grid = nullptr;
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
    m_grid->insertWidget(widget_index, pin_widget);

    Block block;
    block.pinId = id;
    block.pinWidget = pin_widget;
    m_blockList.insert(id,block);
}

void JZVisionSettingDialog::onPinRemove()
{
    auto *pin_widget = dynamic_cast<JZVisionSettingPinWidget*>(sender());
    int id = pin_widget->pinId();
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
    if (m_node->type() == Node_if)
    {
        auto node_if = dynamic_cast<JZNodeIf*>(m_node);
        if(!node_if->hasElse())
            node_if->addElsePin();
    }
    else
    {
        auto node_switch = dynamic_cast<JZNodeSwitch*>(m_node);
        if (!node_switch->hasDefault())
            node_switch->addDefault();
    }
}

void JZVisionSettingDialog::setNode(JZNode* node)
{
    m_node = node;
    QWidget *area_widget = new QWidget();    
    
    QVBoxLayout *v = new QVBoxLayout();
    area_widget->setLayout(v);        

    QLabel *label_name = new QLabel(view()->nodeName(node->id()));
    v->addWidget(label_name);

    //参数
    auto in_list = node->paramInList();
    if (in_list.size() > 0)
    {
        QVBoxLayout*grid = new QVBoxLayout();
        m_grid = grid;
        for (int i = 0; i < in_list.size(); i++)
        {
            JZVisionSettingPinWidget* pin_widget = createPin(node->pin(in_list[i]));            
            grid->addWidget(pin_widget);

            Block block;
            block.pinWidget = pin_widget;
            block.pinId = in_list[i];
            m_blockList.insert(block.pinId, block);
        }
        if (m_node->type() == Node_if || m_node->type() == Node_switch)
        {
            QToolButton* pin_add = new QToolButton();
            QToolButton* pin_else = new QToolButton();
            connect(pin_add, &QToolButton::clicked, this, &JZVisionSettingDialog::onPinAdd);
            connect(pin_else, &QToolButton::clicked, this, &JZVisionSettingDialog::onPinElse);

            QHBoxLayout* pin_l = new QHBoxLayout();
            pin_l->setContentsMargins(0, 0, 0, 0);
            grid->addLayout(pin_l);
        }
        v->addLayout(grid);
    }
    v->addStretch();

    QScrollArea *area = new QScrollArea();
    area->setWidgetResizable(true);
    area->setWidget(area_widget);

    QTabWidget *tab = new QTabWidget();
    tab->addTab(area, "基本参数");
    setCentralWidget(tab);

    resize(480, 600);
}

QWidget *JZVisionSettingDialog::createRow(QString name, QString value)
{
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    QWidget *w = new QWidget();
    l->addWidget(new QLabel(name));
    l->addWidget(new QLabel(value));
    w->setLayout(l);

    return w;
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