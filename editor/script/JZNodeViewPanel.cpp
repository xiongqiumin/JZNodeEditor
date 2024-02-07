#include <QVBoxLayout>
#include "JZNodeViewPanel.h"

// JZNodeViewPanel
JZNodeViewPanel::JZNodeViewPanel(QWidget *widget)
    : QWidget(widget)
{        
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);

    m_lineSearch = new QLineEdit();
    layout->addWidget(m_lineSearch);
    connect(m_lineSearch,&QLineEdit::returnPressed,this,&JZNodeViewPanel::onSearch);

    m_tree = new QTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);

    connect(m_tree, &QTreeWidget::itemClicked, this, &JZNodeViewPanel::onTreeItemClicked);
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &JZNodeViewPanel::onContextMenu);

    layout->addWidget(m_tree);
    setLayout(layout);    
}

JZNodeViewPanel::~JZNodeViewPanel()
{
}

void JZNodeViewPanel::onSearch()
{

}

void JZNodeViewPanel::onTreeItemClicked(QTreeWidgetItem *current, int col)
{

}

void JZNodeViewPanel::onContextMenu(const QPoint &pos)
{

}