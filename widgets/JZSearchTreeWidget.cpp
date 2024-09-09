#include <QVBoxLayout>
#include "JZSearchTreeWidget.h"

JZSearchTreeWidget::JZSearchTreeWidget()
{
    m_tree = nullptr;

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);

    m_lineSearch = new QLineEdit();    
    connect(m_lineSearch, &QLineEdit::returnPressed, this, &JZSearchTreeWidget::onSearch);

    m_tree = new QTreeWidget();

    layout->addWidget(m_lineSearch);
    layout->addWidget(m_tree);
    setLayout(layout);
}

JZSearchTreeWidget::~JZSearchTreeWidget()
{
}

QTreeWidget *JZSearchTreeWidget::tree()
{
    return m_tree;
}

void JZSearchTreeWidget::onSearch()
{
    QString name = m_lineSearch->text();
    filterItem(m_tree->invisibleRootItem(), name);
    m_tree->expandAll();
}

bool JZSearchTreeWidget::filterItem(QTreeWidgetItem *item, QString name)
{
    bool show = false;
    int count = item->childCount();
    bool has_name = item->text(0).contains(name, Qt::CaseInsensitive);

    if (count == 0)
    {
        show = has_name;
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            if (filterItem(item->child(i), name))
                show = true;
        }        
    }
    item->setHidden(!show);
    return show;
}