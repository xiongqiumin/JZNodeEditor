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

//JZVisionSettingPinWidget
JZVisionSettingPinWidget::JZVisionSettingPinWidget()
{
    m_node = nullptr;
    m_pinEditor = nullptr;
    m_linkTip = nullptr;
    m_setting = nullptr;
    m_pinId = -1;

    m_isLink = false;
    m_btnLink = new QToolButton();
    connect(m_btnLink, &QToolButton::clicked, this, &JZVisionSettingPinWidget::onBtnLink);

    QHBoxLayout* l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_btnLink);
    this->setLayout(l);
}

JZVisionSettingPinWidget::~JZVisionSettingPinWidget()
{
}

void JZVisionSettingPinWidget::setPin(JZNode* node, int pin_id)
{
    m_node = node;
    m_pinId = pin_id;

    auto pin = m_node->pin(pin_id);

    QList<int> in_list = m_node->file()->getConnectInput(m_node->id(), pin_id);
    if (in_list.size() > 0)
    {
        m_linkGemo = m_node->file()->getConnect(in_list[0])->from;
        m_isLink = true;
    }

    auto env = m_node->environment();
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

    updatePinWidget();
}

JZVisionSettingDialog *JZVisionSettingPinWidget::setting()
{
    return m_setting;
}

void JZVisionSettingPinWidget::setSetting(JZVisionSettingDialog *dlg)
{
    m_setting = dlg;
}

void JZVisionSettingPinWidget::updatePinWidget()
{
    auto env = m_node->environment();

    QHBoxLayout *l = qobject_cast<QHBoxLayout*>(layout());
    if (!m_isLink)
    {
        if (m_linkTip)
        {
            delete m_linkTip;
            m_linkTip = nullptr;
        }
        if (!m_pinEditor)
        {
            auto pin = m_node->pin(m_pinId);
            QString up_type = env->upType(pin->dataType());
            JZParamEditInfo info = JZParamEditInfo::createType(env, up_type);
            m_pinEditor = new JZNodeParamValueWidget();
            m_pinEditor->init(info);
            l->insertWidget(0, m_pinEditor);
            m_pinEditor->setValue(m_node->pinValue(m_pinId));
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
        if (!m_linkTip)
        {                        
            m_linkTip = new QComboBox();
            updateLinkList();
            l->insertWidget(0, m_linkTip);
            if (m_linkGemo.isNull())
                m_linkTip->setCurrentIndex(-1);
            else
            {
                int idx = m_linkTip->findData(m_linkGemo.paramId());
                m_linkTip->setCurrentIndex(idx);
            }
        }
        m_btnLink->setText("直接输入");
    }
}

void JZVisionSettingPinWidget::addLinkItem(JZNode* node, const QList<int> &dst_types)
{
    QTreeWidgetItem *root = nullptr;

    auto env = node->environment();
    auto out_list = node->paramOutList();
    for (int i = 0; i < out_list.size(); i++)
    {
        int out_pin = out_list[i];
        QList<int> src_types = env->nameListToTypeList(node->pinType(out_pin));
        if (env->matchType(src_types, dst_types) != Type_none)
        {            
            JZNodeGemo from(node->id(), out_pin);
            QString from_name = m_setting->view()->pinName(from);
            m_linkTip->addItem(from_name, from.paramId());
        }
    }
}

void JZVisionSettingPinWidget::updateLinkList()
{
    auto env = m_node->environment();
    QList<int> pin_type = env->nameListToTypeList(m_node->pinType(m_pinId));

    JZScriptItemVisitor visitor(m_node->file());
    QList<JZNode*> in_list = visitor.flowInputNodeRecursively(m_node);
    for (int i = 0; i < in_list.size(); i++)
    {
        addLinkItem(in_list[i], pin_type);
    }
}

void JZVisionSettingPinWidget::onBtnLink()
{
    m_isLink = !m_isLink;
    updatePinWidget();
}

bool JZVisionSettingPinWidget::isLink()
{
    return m_isLink;
}

JZNodeGemo JZVisionSettingPinWidget::linkGemo()
{
    if (m_isLink)
    {
        int paramId = m_linkTip->currentData().toInt();
        return JZNodeGemo::fromParamId(paramId);
    }
    else
        return JZNodeGemo();
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

void JZVisionSettingDialog::setNode(JZNode* node)
{
    m_node = node;
    QWidget *area_widget = new QWidget();    
    
    QVBoxLayout *v = new QVBoxLayout();
    area_widget->setLayout(v);        

    QLabel *label_name = new QLabel(view()->nodeName(node));
    v->addWidget(label_name);

    auto in_list = node->paramInList();
    if (in_list.size() > 0)
    {
        QGridLayout *grid = new QGridLayout();
        for (int i = 0; i < in_list.size(); i++)
        {
            QLabel *pin_label = new QLabel(node->pinName(in_list[i]));

            JZVisionSettingPinWidget* pin_widget = createPin(node->pin(in_list[i]));            

            grid->addWidget(pin_label, i, 0);
            grid->addWidget(pin_widget, i, 1);

            Block block;
            block.pinWidget = pin_widget;
            block.pinId = in_list[i];
            m_blockList.insert(block.pinId, block);
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