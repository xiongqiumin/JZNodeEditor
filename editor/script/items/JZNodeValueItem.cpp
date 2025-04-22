#include <QComboBox>
#include <QPushButton>
#include "JZNodeValueItem.h"
#include "JZNodeValue.h"
#include "JZNodeFactory.h"

//JZNodeLiteralItem
JZNodeLiteralItem::JZNodeLiteralItem()
{
}

void JZNodeLiteralItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    auto out_id = m_node->paramOut(0);
    auto block = m_blocks[out_id].data();
    QString data_type = m_node->pinType(out_id)[0];
    m_title = data_type;
    
    block->isShowName = false;
    if(data_type == "null")    
        block->isShowValue = false;
    else
        block->isShowValue = true;
    
    block->isEditable = block->isShowValue;
}

//JZNodeParamItem
JZNodeParamItem::JZNodeParamItem()
{
}

void JZNodeParamItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    auto out_id = m_node->paramOut(0);
    auto block = m_blocks[out_id].data();
    block->isShowValue = true;
    block->isEditable = true;
}

//JZNodeSetParamItem
JZNodeSetParamItem::JZNodeSetParamItem()
{

}

//JZNodeEnumItem
JZNodeEnumItem::JZNodeEnumItem()
{
}

void JZNodeEnumItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    auto out_id = m_node->paramOut(0);
    auto block = m_blocks[out_id].data();
    QString data_type = m_node->pinType(out_id)[0];
    m_title = data_type;
    block->isShowName = false;
    block->isShowValue = true;
}

//JZNodeFlagItem
JZNodeFlagItem::JZNodeFlagItem()
{
}

void JZNodeFlagItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    auto out_id = m_node->paramOut(0);
    auto block = m_blocks[out_id].data();
    QString data_type = m_node->pinType(out_id)[0];
    m_title = data_type;
    block->isShowName = false;
    block->isShowValue = true;
}