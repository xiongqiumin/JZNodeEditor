#include "JZNodeValidator.h"
#include "JZNodeFlow.h"

JZNodeValidatorManager *JZNodeValidatorManager::instance()
{
    static JZNodeValidatorManager inst;
    return &inst;
}

JZNodeValidatorManager::JZNodeValidatorManager()
{
    JZParamValidator for_op;
    for_op.type = JZParamValidator::enumeration;
    for_op.enumList << "<"  << "<=" << ">" << ">=" << "==" << "!=";

    registPinValidator(Node_for,JZNodeFor::Pin_op,for_op);
}

void JZNodeValidatorManager::registPinValidator(int node_type,int pin_id,JZParamValidator validator)
{
}

const JZParamValidator *JZNodeValidatorManager::pinValidator(int node_type,int pin_id)
{
    if(!m_pinValidator.contains(node_type))
        return nullptr;

    if(!m_pinValidator[node_type].contains(pin_id))
        return nullptr;

    return &m_pinValidator[node_type][pin_id];
}