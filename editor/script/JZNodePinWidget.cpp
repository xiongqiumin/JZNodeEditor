#include <QHBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QPushButton>
#include <QLineEdit>
#include "JZNode.h"
#include "JZNodeObject.h"
#include "JZNodePinWidget.h"
#include "JZNodeType.h"
#include "JZNodeFlagEditDialog.h"
#include "JZNodeTypeHelper.h"
#include "JZScriptEnvironment.h"
#include "JZNodeParamEditWidget.h"
#include "JZNodeParamDisplayWidget.h"

//JZNodePinWidget
JZNodePinWidget::JZNodePinWidget(JZNode *node, int pin_id)    
{
    m_node = node;
    m_pin = pin_id;
}

JZNodePinWidget::~JZNodePinWidget()
{
}

QString JZNodePinWidget::value() const
{
    return QString();
}

void JZNodePinWidget::setValue(const QString &)
{
}

void JZNodePinWidget::updateWidget()
{
}

//JZNodePinButtonWidget
JZNodePinButtonWidget::JZNodePinButtonWidget(JZNode *node, int pin_id)
    :JZNodePinWidget(node, pin_id)
{
    QVBoxLayout *lay = new QVBoxLayout();
    lay->setContentsMargins(0, 0, 0, 0);
    
    m_btn = new QPushButton();    
    lay->addWidget(m_btn);
    this->setLayout(lay);
    m_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

JZNodePinButtonWidget::~JZNodePinButtonWidget()
{

}

QPushButton *JZNodePinButtonWidget::button()
{
    return m_btn;
}

//JZNodePinValueWidget
JZNodePinValueWidget::JZNodePinValueWidget(JZNode *node, int pin_id)
    :JZNodePinWidget(node, pin_id)
{
    m_widget = new JZNodeParamValueWidget();
    connect(m_widget, &JZNodeParamValueWidget::sigValueChanged, this, &JZNodePinValueWidget::onValueChanged);

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_widget);
    setLayout(l);        
}

JZNodePinValueWidget::~JZNodePinValueWidget()
{

}

void JZNodePinValueWidget::onValueChanged()
{    
    emit sigValueChanged(value());    
}

void JZNodePinValueWidget::initWidget(int data_type, QString widget_type)
{
    m_widget->initWidget(data_type, widget_type);
}

QWidget *JZNodePinValueWidget::focusWidget()
{
    return m_widget->focusWidget();
}

QString JZNodePinValueWidget::value() const
{
    return m_widget->value();
}

void JZNodePinValueWidget::setValue(const QString &value)
{
    m_widget->setValue(value);
}

//JZNodePinDisplayWidget
JZNodePinDisplayWidget::JZNodePinDisplayWidget(JZNode *node, int pin_id)
    :JZNodePinWidget(node,pin_id)
{
    m_widget = nullptr;

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);        
    m_dataType = Type_none;

    createWidget();
}

void JZNodePinDisplayWidget::createWidget()
{    
    auto env = editorEnvironment();
    int need_type = Type_none;
    if (m_node)
    {
        auto file = m_node->file();
        auto list = file->getConnectPin(m_node->id(), m_pin);
        if (list.size() == 1)
        {
            auto line = file->getConnect(list[0]);
            auto type_list = env->nameToTypeList(file->getPin(line->from)->dataType());
            if (type_list.size() == 1)
                need_type = type_list[0];
        }
    }
    if (m_dataType == need_type)
        return;    

    if (m_widget)
        delete m_widget;

    m_dataType = need_type;
    auto d = editorEnvironment()->editorManager()->delegate(m_dataType);;
    if (d && d->createDisplay)
    {        
        m_widget = d->createDisplay();
        resize(m_widget->size());
    }
    else
    {        
        auto line = new QLineEdit();
        line->setReadOnly(true);
        m_widget = line;
        resize(120, 24);
    }    
    m_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    layout()->addWidget(m_widget);
}

void JZNodePinDisplayWidget::updateWidget()
{
    createWidget();
    emit sigSizeChanged(size());
}

void JZNodePinDisplayWidget::setRuntimeValue(const JZNodeDebugParamValue &value)
{
    if (m_widget->inherits("QLineEdit"))
    {
        auto line = qobject_cast<QLineEdit*>(m_widget);
        line->setText(value.value);
    }
    else if (m_widget->inherits("JZNodeParamDisplayWidget"))
    {
        auto display = qobject_cast<JZNodeParamDisplayWidget*>(m_widget);
        display->setRuntimeValue(value);
    }    
}