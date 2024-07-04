#include <memory>
#include <JZRegExpHelp.h>
#include "JZNodeEngine.h"
#include "JZNodeObjectParser.h"

JZNodeObjectParser::JZNodeObjectParser()
{
    m_col = m_line = -1;
    m_currentIndex = 0;
    m_gapList << ',' << ':' << '[' << ']' << '{' << '}';
}

JZNodeObjectParser::~JZNodeObjectParser() 
{        
}

void JZNodeObjectParser::iniContext(const QString &text)
{
    m_content = text;
    m_col = 0;
    m_line = 0;
    m_currentIndex = 0;
    m_error.clear();
}

void JZNodeObjectParser::makeError(const QString &error)
{
    m_error = "row: " + QString::number(m_line + 1) + ",col: " + QString::number(m_col + 1) + " " + error; 
}

void JZNodeObjectParser::makeExpectError(QString text,QString give)
{
    QString error = QString("expect '%1' but give '%2'").arg(text,give);
    makeError(error);
}

bool JZNodeObjectParser::checkVariable(const QVariant &v,int type)
{
    int v_type = JZNodeType::variantType(v);
    if(v_type == Type_none)  //前面解析已经出错，直接返回
        return false;

    if(!JZNodeType::canConvert(v_type,type))
    {
        QString error = "type error, need " + JZNodeType::typeToName(type) + ", but give " + JZNodeType::typeToName(v_type);
        makeError(error);
        return false;
    }

    return true;
}

QString JZNodeObjectParser::error()
{
    return m_error;
}

QChar JZNodeObjectParser::nextChar()
{
    int index = m_currentIndex;
    while (index < m_content.size())
    {
        QChar c = m_content[index++];        
        if (!c.isSpace())        
            return c;        
    }
    return EOF;
}

QChar JZNodeObjectParser::readChar()
{
    while (m_currentIndex < m_content.size())
    {
        QChar c = m_content[m_currentIndex++];
        m_col++;

        if (!c.isSpace())
        {
            return c;
        }
        else if (c == '\r')
        {
            m_col = 0;
            m_line++;
        }                
    }
    return EOF;
}

QString JZNodeObjectParser::readWord()
{
    QString word;
    while (m_currentIndex < m_content.size())
    {
        QChar c = m_content[m_currentIndex++];
        if (c.isSpace())
        {
            if (c == '\n')
            {
                m_col = 0;
                m_line++;
            }
            if (!word.isEmpty())
                break;
        }
        else if (c == '<')
        {
            word.push_back(c);
            while(true)
            {
                c = readChar();
                word.push_back(c);
                if(c == '>')
                    break;
                if(m_currentIndex == m_content.size())
                    return word;
            }
        }
        else if (m_gapList.contains(c))
        {
            m_currentIndex--;
            break;
        }
        else
        {
            word.push_back(c);
        }
        m_col++;
    }
    return word;
}

JZList *JZNodeObjectParser::readList(QString valueType,QChar gap)
{    
    QChar c = readChar();
    Q_ASSERT(c == gap);
    gap = (gap == '[')? ']':'}';

    int data_type = JZNodeType::nameToType(valueType);
    QScopedPointer<JZList> ptr(new JZList());
    ptr->valueType = valueType;
    while (1)
    {
        QVariant v = readVariable();
        if(!checkVariable(v,data_type))
            return nullptr;

        v = JZNodeType::convertTo(data_type,v);
        ptr->list.push_back(v);

        c = readChar();
        if (c == gap)
            break;
       
        if (c != ',')
        {
            makeExpectError(",",c);
            return nullptr;
        }
    }

    return ptr.take();    
}

JZMap *JZNodeObjectParser::readMap(QString keyType,QString valueType)
{
    QChar c = readChar();
    Q_ASSERT(c == '{');

    int key_type = JZNodeType::nameToType(keyType);
    int value_type = JZNodeType::nameToType(valueType);

    QScopedPointer<JZMap> ptr(new JZMap());
    ptr->keyType = keyType;
    ptr->valueType = valueType;
    while (1)
    {
        QVariant key = readVariable();
        if(!checkVariable(key,key_type))
            return nullptr;
        key = JZNodeType::convertTo(key_type,key);
        
        c = readChar();
        if (c != ':')
        {
            makeExpectError(":",c);
            return nullptr;        
        }

        QVariant value = readVariable();
        if(!checkVariable(value,value_type))
            return nullptr;
        value = JZNodeType::convertTo(key_type,value);

        JZMap::Key map_key;
        map_key.v = key;
        ptr->map.insert(map_key, value);

        c = readChar();
        if (c == '}')
            break;
        if (c != ',')
        {
            makeExpectError(",",c);
            return nullptr;
        }
    }

    return ptr.take();    
}

JZNodeObject *JZNodeObjectParser::readObject()
{    
    auto inst = JZNodeObjectManager::instance();
    QString type = readWord();
    if (type.isEmpty())
    {
        QChar c = nextChar();
        if(c == '{')
        {
            auto map = readMap("string","any");
            if(map)
                return inst->createRefrence(map->type(),map,true);
        }
        else if(c == '[')
        {
            auto list = readList("any",'[');
            if(list)
                return inst->createRefrence(list->type(),list,true);
        }

        return nullptr;
    }

    if(!JZRegExpHelp::isWord(type))
    {
        m_error = "error format " + type;
        return nullptr;
    }
    if(nextChar() != '{')
    {
        makeExpectError("{", nextChar());
        return nullptr;
    }
    
    auto meta = JZNodeObjectManager::instance()->meta(type);
    if (!meta)
    {
        makeError("no object " + type);
        return nullptr;
    }

    if(type.startsWith("List<"))
    {
        int end_idx = type.lastIndexOf(">");
        QString value_type = type.mid(5,end_idx - 5);
        auto list = readList(value_type,'{');
        if(list)
            return inst->createRefrence(list->type(),list,true);
        return nullptr;
    }
    else if(type.startsWith("Map<"))
    {
        int end_idx = type.lastIndexOf(">");
        QStringList type_list = type.mid(4,end_idx - 4).split(",");
        QString key_type = type_list[0];
        QString value_type = type_list[1];
        auto map = readMap(key_type,value_type);
        if(map)
            return inst->createRefrence(map->type(),map,true); 
        return nullptr;
    }

    auto func_def = meta->function("__fromString__");
    if (func_def)
    {
        QString create_string;
        if (!readBkt(create_string))
            return nullptr;

        QVariantList in,out;
        in << create_string;
        JZScriptInvoke(func_def->fullName(), in, out);

        auto ptr = toJZObjectPtr(out[0]);
        ptr.releaseOwner();
        return ptr.object();
    }
    else
    {
        JZNodeObject *obj = JZNodeObjectManager::instance()->create(meta->id);
        QScopedPointer<JZNodeObject> ptr(obj);
        QScopedPointer<JZMap> map(readMap("string","any"));        
        if (!map)
            return nullptr;

        auto it = map->map.begin();
        while (it != map->map.end())
        {
            QString param_name = it.key().v.toString();
            auto param_def = obj->meta()->param(param_name);
            if (!param_def)
            {
                makeError("no param " + param_name);
                return nullptr;
            }

            int param_type = param_def->dataType();
            if (!checkVariable(it.value(),param_type))
                return nullptr;
            
            obj->setParam(param_name,JZNodeType::convertTo(param_type, it.value()));
            it++;
        }
        return ptr.take();   
    }
}

QVariant JZNodeObjectParser::readVariable()
{
    QChar c = nextChar();

    JZNodeObject *obj = nullptr;
    if (c.isDigit() || c == '-')
    {
        QString text = readWord();
        if (JZRegExpHelp::isInt(text))
            return text.toInt();
        else if (JZRegExpHelp::isHex(text))
            return text.toInt(nullptr, 16);
        else if (JZRegExpHelp::isFloat(text))
            return text.toDouble();
        else
            return QVariant();
    }
    else if(c == '"')
    {
        QString text;
        if(readString(text))
            return text;
        else
            return QVariant();
    }
    else
    {
        obj = readObject();
        if (!obj)
            return QVariant();
        return QVariant::fromValue(JZNodeObjectPtr(obj,true));
    }    
   
}

bool JZNodeObjectParser::checkIsEnd()
{    
    return (nextChar() == EOF);
}

bool JZNodeObjectParser::readString(QString &text)
{
    QChar c = readChar();
    Q_ASSERT(c == '\"');

    QString word;
    while (m_currentIndex < m_content.size())
    {
        QChar c = m_content[m_currentIndex++];
        m_col++;

        if (c == '\\')
        {
            if (m_currentIndex < m_content.size() - 2
                && m_content[m_currentIndex] == '0'
                && m_content[m_currentIndex].toLower() == 'x')
            {                
            }
            else if (m_currentIndex < m_content.size())
            {                
            }
        }
        else if (c == '\"')
        {
            text = word;
            return true;
        }

        word.push_back(c);
    }

    makeError("except '\"'");
    return false;
}

bool JZNodeObjectParser::readBkt(QString &context)
{
    if (readChar() != '{')
        return false;

    int start = m_currentIndex;
    int bkt_num = 1;
    QString word;
    while (m_currentIndex < m_content.size())
    {
        QChar c = m_content[m_currentIndex++];
        m_col++;

        if (c == '{')
            bkt_num++;
        else if (c == '}')
        {
            bkt_num--;
            if (bkt_num == 0)
            {
                context = word;
                return true;
            }
        }
        else if (c == '\"')
        {
            QString tmp;
            if (!readString(tmp))
                return false;
        }        
        word.push_back(c);
    }
    return false;

}

JZNodeObject *JZNodeObjectParser::parse(const QString &text)
{             
    iniContext(text);

    JZNodeObject *obj = readObject();
    if(!obj)
        return nullptr;
    
    if (!checkIsEnd())
    {   
        m_error = "no expect char '" + QString(nextChar()) + "'";
        delete obj;
        return nullptr;
    }
    
    return obj;
}

JZNodeObject *JZNodeObjectParser::parseToType(QString type,const QString &text)
{
    QString type_text = type + "{" + text +"}";
    return parse(type_text);
}

//JZNodeObjectFormat
JZNodeObjectFormat::JZNodeObjectFormat()
{
}

JZNodeObjectFormat::~JZNodeObjectFormat()
{

}

QString JZNodeObjectFormat::variantToString(const QVariant &v)
{
    int type = JZNodeType::variantType(v);
    if (JZNodeType::isObject(type))
        return objectToString(toJZObject(v));
    else    
        return JZNodeType::convertTo(Type_string, v).toString();    
}

QString JZNodeObjectFormat::listToString(const JZList *list)
{
    QString context;
    if(list->type() != "Map<string,any>")
        context = list->type() + "{";
    else
        context = "[";
    for (int i = 0; i < list->list.size(); i++)
    {
        context += variantToString(&list->list.at(i));
        if (i != list->list.size() - 1)
            context += ",";
    }
    if(list->type() != "Map<string,any>")
        context += "}";
    else
        context += "]";

    return context;         
}

QString JZNodeObjectFormat::mapToString(const JZMap *map)
{
    QString context;
    if(map->type() != "Map<string,any>")
        context += map->type();

    context += "{";
    auto it = map->map.begin();
    while (it != map->map.end())
    {
        QString name = variantToString(it.key().v);
        QString value = variantToString(it.value());
        context += name + ":" + value;

        it++;
        if(it != map->map.end())
            context += ",";
    }
    context += "}";
    return context;
}

QString JZNodeObjectFormat::objectToString(JZNodeObject *obj)
{    
    QString text;
    if(JZObjectIsList(obj))
    {
        JZList *list = (JZList *)obj->cobj();
        text = listToString(list);
    }
    else if(JZObjectIsMap(obj))
    {
        JZMap *map = (JZMap *)obj->cobj();
        text = mapToString(map);
    }
    else
    {
        text = obj->className() + "{";
        auto func_def = obj->function("__toString__");
        if (func_def)
        {        
            QVariantList in, out;
            in << QVariant::fromValue(JZNodeObjectPtr(obj,false));
            JZScriptInvoke(func_def->fullName(), in, out);
            text += out[0].toString();
        }
        else
        {
            auto params = obj->paramList();
            for (int i = 0; i < params.size(); i++)
            {
                QString name = "\"" + params[i] + "\"";
                QString value = variantToString(obj->param(params[i]));
                text += name + ":" + value;
                if (i != params.size() - 1)
                    text += ",";
            }
        }
        text += "}";
    }
    return text;
}

QString JZNodeObjectFormat::format(JZNodeObject *obj)
{
    return objectToString(obj);
}
