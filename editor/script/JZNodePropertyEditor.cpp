#include "JZNodePropertyEditor.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>

JZNodePropertyEditor::JZNodePropertyEditor(QWidget *widget)
    :QWidget(widget)
{
    m_node = nullptr;
    m_tree = new QtTreePropertyBrowser();

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->addWidget(m_tree);
    setLayout(l);

    m_propManager = new QtVariantPropertyManager();
    m_propEditor = new QtVariantEditorFactory();

    m_tree->setFactoryForManager(m_propManager, m_propEditor);
}

JZNodePropertyEditor::~JZNodePropertyEditor()
{
}

void JZNodePropertyEditor::onPropUpdate()
{
    emit sigPropUpdate(m_node->id());
}

JZNode *JZNodePropertyEditor::node()
{
    return m_node;
}

void JZNodePropertyEditor::setNode(JZNode *node)
{
    m_node = node;
    m_tree->clear();
    m_propManager->clear();
    if(!m_node)
        return;

    auto prop_base = m_propManager->addProperty(m_propManager->groupTypeId(), "基本信息");
    auto prop_name = m_propManager->addProperty(QVariant::String, "名称");
    prop_base->addSubProperty(prop_name);
    prop_name->setValue(m_node->name());
    m_tree->addProperty(prop_base);

    auto in_list = node->propInList(Prop_param);
    if(in_list.size() > 0)
    {
        auto prop_in = m_propManager->addProperty(m_propManager->groupTypeId(), "输入");
        for(int i = 0; i < in_list.size(); i++)
        {
            auto prop = m_propManager->addProperty(QVariant::String, node->prop(in_list[i])->name());
            prop->setEnabled(false);
            prop_in->addSubProperty(prop);
        }
        m_tree->addProperty(prop_in);
    }

    auto out_list = node->propOutList(Prop_param);
    if(out_list.size() > 0)
    {
        auto prop_out = m_propManager->addProperty(m_propManager->groupTypeId(), "输出");
        for(int i = 0; i < out_list.size(); i++)
        {
            auto prop = m_propManager->addProperty(QVariant::String, node->prop(out_list[i])->name());
            prop_out->addSubProperty(prop);
        }
        m_tree->addProperty(prop_out);
    }
}
