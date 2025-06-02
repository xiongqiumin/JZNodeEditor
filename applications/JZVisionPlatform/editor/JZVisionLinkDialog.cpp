#include <QTabWidget>
#include <QVBoxLayout>
#include "JZVisionLinkDialog.h"
#include "JZScriptItemVisitor.h"
#include "JZScriptEnvironment.h"

JZVisionLinkDialog::JZVisionLinkDialog(QWidget *parent) 
    : JZBaseDialog(parent)
{        
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);
    m_tree->setSelectionMode(QTreeWidget::SingleSelection);

    l->addWidget(m_tree);
    centralWidget()->setLayout(l);

    resize(320, 480);
}

JZVisionLinkDialog::~JZVisionLinkDialog()
{
}

void JZVisionLinkDialog::addLinkItem(JZNode* node, const QList<int> &dst_types)
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
            if (!root)
            {
                root = new QTreeWidgetItem();
                root->setText(0,node->name());
                root->setFlags(root->flags() & ~Qt::ItemIsSelectable);
                m_tree->addTopLevelItem(root);
            }

            QTreeWidgetItem *link = new QTreeWidgetItem();
            root->addChild(link);
            link->setText(0,node->pinName(out_pin));
            link->setData(0, Qt::UserRole, JZNodeGemo::paramId(node->id(), out_pin));
        }
    }
}

void JZVisionLinkDialog::setNode(JZNode* node,int pin_id)
{
    auto env = node->environment();
    QList<int> pin_type = env->nameListToTypeList(node->pinType(pin_id));

    JZScriptItemVisitor visitor(node->file());
    QList<JZNode*> in_list = visitor.flowInputNodeRecursively(node);
    for (int i = 0; i < in_list.size(); i++)
    {
        addLinkItem(in_list[i], pin_type);
    }

    m_tree->expandAll();
}

void JZVisionLinkDialog::accept()
{
    auto item = m_tree->currentItem();
    if (!item)
    {
        return;
    }

    int param_id = item->data(0, Qt::UserRole).toInt();    
    m_result = JZNodeGemo::fromParamId(param_id);
    JZBaseDialog::accept();
}

JZNodeGemo JZVisionLinkDialog::result()
{
    return m_result;
}