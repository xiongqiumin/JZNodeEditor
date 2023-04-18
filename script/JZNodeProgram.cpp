#include "JZNodeProgram.h"
#include "JZNodeCompiler.h"

//JZEventHandle
JZEventHandle::JZEventHandle()
{    
}

bool JZEventHandle::match(JZEvent *event) const
{
    if(event->eventType() == type && event->params == params)
        return true;

    return false;
}

//JZNodeProgram
JZNodeProgram::JZNodeProgram()
{
    m_opNames = QStringList{"+","-","*","%","%","==","!=",">",">=","<","<=","&&","||","^"};
}

JZNodeProgram::~JZNodeProgram()
{
}

bool JZNodeProgram::load(QString file)
{
    return false;
}

const QList<JZEventHandle> &JZNodeProgram::eventHandleList() const
{
    return m_events;
}

int JZNodeProgram::getPc(int nodeId)
{
    return 0;
}

QString JZNodeProgram::paramName(int id)
{
    return JZNodeCompiler::paramName(id);
}

QString JZNodeProgram::dump()
{    
    QString content;
    
    for(int i = 0; i < opList.size(); i++)
    {
        //deal op
        JZNodeIR &op = opList[i];
        QString line;
        switch (op.type)
        {
        case OP_nop:
        {
            line = "NOP";
            break;
        }
        case OP_set:
        {
            line = "SET reg" + paramName(op.params[0].toInt()) + " = " + paramName(op.params[1].toInt());
            break;
        }
        case OP_setValue:
        {
            line = "SET reg" + paramName(op.params[0].toInt()) + " = " + op.params[1].toString();
            break;
        }
        case OP_get:
        {
            Q_ASSERT(0);
            break;
        }
        case OP_call:
        {
            line = "CALL ";
            line += op.params[0].toString();
            break;
        }
        case OP_return:
            line = "RETURN";
            break;
        case OP_exit:
            line = "EXIT";
            break;
        case OP_add:                        
        case OP_sub:            
        case OP_mul:                
        case OP_div:
        case OP_mod:                
        case OP_eq:            
        case OP_ne:            
        case OP_le:            
        case OP_ge:            
        case OP_lt:            
        case OP_gt:
        case OP_and:
        case OP_or:
        case OP_xor:
        {
            QString c = paramName(op.params[0].toInt());
            QString a = paramName(op.params[1].toInt());
            QString b = paramName(op.params[2].toInt());
            line = c + " = " + a + " " + m_opNames[op.type - OP_add] + " " + b;
            break;
        }
        case OP_jmp:
            line = "JMP " + op.params[0].toString();
            break;
        default:
            Q_ASSERT(0);
            break;
        }
        line = line.leftJustified(12);
        if(!op.memo.isEmpty())
            line += " #" + op.memo;
        content += line + "\n";
    }
    return content;
}
