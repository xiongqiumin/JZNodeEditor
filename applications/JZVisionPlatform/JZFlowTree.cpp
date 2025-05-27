#include "JZFlowTree.h"


JZFlowTree::JZFlowTree()
{ 
}


JZFlowTree::~JZFlowTree()
{
}

void JZFlowTree::init()
{
    m_tree->clear();
    if (!m_project)
        return;

    auto class_item = m_project->getClass("JZVisionPlatform");

    auto root = addItem(m_tree->invisibleRootItem(), class_item);
    root->setText(0, "Application");
    sortItem(root);

    auto init_item = getViewItem(class_item->memberFunction("init"));
    init_item->setHidden(true);

    m_tree->expandAll();
}