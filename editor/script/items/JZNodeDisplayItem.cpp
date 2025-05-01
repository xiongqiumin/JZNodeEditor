#include <QComboBox>
#include <QPushButton>
#include "JZNodeDisplayItem.h"
#include "JZNodeValue.h"

//JZNodeDisplayItem
JZNodeDisplayItem::JZNodeDisplayItem()
{
    QPushButton *btnAdd = new QPushButton("Add");
    btnAdd->connect(btnAdd, &QPushButton::clicked, [this] {
        this->onAddClicked();
    });
    
    m_addBlock = createWidgetBlock(btnAdd,true);
    m_addBlock->pri = 8;   
}

void JZNodeDisplayItem::updatePin()
{
    JZNodeGraphItem::updatePin();
}

void JZNodeDisplayItem::setValue(int pin,QVariantPtr *ref)
{
    JZNodeDisplay *node_display = (JZNodeDisplay *)m_node;

    QByteArray oldValue = saveNode();
    node_display->addInput();
    notifyPropChanged(oldValue);
}

void JZNodeDisplayItem::onAddClicked()
{
    JZNodeDisplay *node_display = (JZNodeDisplay *)m_node;
    node_display->addInput();
}