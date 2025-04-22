#include "JZScriptItemUnitTest.h"

//JZNodeUnitTest
JZNodeUnitTest::JZNodeUnitTest()
{
}

JZNodeUnitTest::~JZNodeUnitTest()
{
}

//JZScriptItemUnitTest
JZScriptItemUnitTest::JZScriptItemUnitTest()
{
    m_script = nullptr;
}

JZScriptItemUnitTest::~JZScriptItemUnitTest()
{
}

void JZScriptItemUnitTest::init(JZScriptItem *script)
{
    m_script = script;

    JZScriptItemVistor visitor;
    visitor.init(m_script); 
}

void JZScriptItemUnitTest::applyDepends(const JZScriptItemDepend &depend)
{
    JZNode *src_node;
    JZNode *dst_node;
    auto pin_list = src_node->pinList();



    for(int i = 0;  i< pin_list.size(); i++)
    {
        JZNodePin pin;
        pin = *src_node->pin(pin_list[i]);
    }
}