#include <QVBoxLayout>
#include "flowTreeWidget.h"

//JZFlowTreeWidget
JZFlowTreeWidget::JZFlowTreeWidget(QWidget* parent)
{    
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree, &JZFlowTreeWidget::customContextMenuRequested, this, &JZFlowTreeWidget::onContexMenu);

    l->addWidget(m_tree);
    setLayout(l);
}

JZFlowTreeWidget::~JZFlowTreeWidget()
{
}

void JZFlowTreeWidget::onContexMenu(QPoint pt)
{
    auto item = m_tree->itemAt(pt);
    if (!item)
        return;

}