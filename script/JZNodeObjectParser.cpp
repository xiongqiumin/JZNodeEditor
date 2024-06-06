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
}

QString JZNodeObjectParser::error()
{
    return QString();
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
        m_col++;

        if (c.isSpace() || m_gapList.contains(c))
        {
            if (c == '\r')
            {
                m_col = 0;
                m_line++;
            }
            if (m_gapList.contains(c))
                m_currentIndex--;

            if (word.isEmpty())
                continue;
            else
                break;
        }
        word.push_back(c);
    }
    return word;
}

QVariantList *JZNodeObjectParser::readList()
{    
    if (readChar() != "[")
        return nullptr;

    QScopedPointer<QVariantList> ptr(new QVariantList());
    while (1)
    {
        QVariant v = readVariable();
        if (JZNodeType::variantType(v) == Type_none)
            return nullptr;
        ptr->push_back(v);

        QChar c = readChar();
        if (c == ']')
            break;
        if (c != ',')
            return nullptr;
    }

    return ptr.take();    
}

QVariantMap *JZNodeObjectParser::readMap()
{
    if (readChar() != '{')
        return nullptr;

    QScopedPointer<QVariantMap> ptr(new QVariantMap());
    while (1)
    {
        JZNodeObjectManager::instance()->create(Type_map);
        QString name;
        if (!readString(name))
            return nullptr;

        QChar c = readChar();
        if (c != ':')
            return nullptr;        

        QVariant v = readVariable();
        if (JZNodeType::variantType(v) == Type_none)
            return nullptr;
        ptr->insert(name, v);

        c = readChar();
        if (c == '}')
            break;
        if (c != ',')
            return nullptr;
    }

    return ptr.take();    
}

JZNodeObject *JZNodeObjectParser::readObject()
{    
    QString type = readWord();
    if (type.isEmpty())
        return nullptr;

    auto meta = JZNodeObjectManager::instance()->meta(type);
    if (!meta)
        return nullptr;

    JZNodeObject *obj = JZNodeObjectManager::instance()->create(meta->id);
    QScopedPointer<JZNodeObject> ptr(obj);
    auto func_def = meta->function("fromString");
    if (func_def)
    {
        QString create_string;
        if (!readBkt(create_string))
            return nullptr;

        QVariantList in,out;
        in << QVariant::fromValue(obj) << create_string;
        JZScriptInvoke(func_def->fullName(), in, out);
    }
    else
    {
        QScopedPointer<QVariantMap> map(readMap());        
        if (!map)
            return nullptr;

        auto it = map->begin();
        while (it != map->end())
        {
            auto param_def = obj->meta()->param(it.key());
            if (!param_def)
                return nullptr;

            int v_type = JZNodeType::variantType(it.value());
            if (!JZNodeType::canConvert(v_type, param_def->dataType()))
                return nullptr;
            
            obj->setParam(it.key(),it.value());
            it++;
        }
    }

    return ptr.take();    
}

QVariant JZNodeObjectParser::readVariable()
{
    QChar c = nextChar();

    JZNodeObject *obj = nullptr;
    if (c == '{')
    {        
        QVariantMap *map = readMap();
        if (!map)
            return QVariant();

        obj = JZNodeObjectManager::instance()->createCClassRefrence(Type_list, map, true);

    }
    else if (c == '[')
    {
        QVariantList *list = readList();
        if (!list)
            return false;
        obj = JZNodeObjectManager::instance()->createCClassRefrence(Type_list, list, true);
    }
    else if(c == '\"')
    {
        QString text;
        if (!readString(text))
            return QVariant();

        return text;
    }
    else if (c.isDigit())
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
    else
    {
        obj = readObject();
        if (!obj)
            return QVariant();
    }    
    
    return QVariant::fromValue(JZNodeObjectPtr(obj));
}

bool JZNodeObjectParser::checkIsEnd()
{    
    return (nextChar() == EOF);
}

bool JZNodeObjectParser::readString(QString &text)
{
    if (readChar() != '\"')
        return false;

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
            break;
        word.push_back(c);
    }

    text = word;
    return true;
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

    JZNodeObject *obj = nullptr;
    QChar c = nextChar();
    if (c == '{')
    {
        QVariantMap *map = readMap();
        if (!map)
            return nullptr;
        
        obj = JZNodeObjectManager::instance()->createCClassRefrence(Type_map, map, true);        
    }
    else if (c == '[')
    {
        QVariantList *list = readList();
        if (!list)
            return nullptr;
        obj = JZNodeObjectManager::instance()->createCClassRefrence(Type_list, list, true);
    }
    else if(c.isLetterOrNumber())
    {
        obj = readObject();
        if (!obj)
            return nullptr;
    }
    else 
    {   
        return nullptr;
    }
    
    if (!checkIsEnd())
    {
        delete obj;
        return nullptr;
    }
    
    return obj;
}

//JZNodeObjectFormat
JZNodeObjectFormat::JZNodeObjectFormat()
{
}

JZNodeObjectFormat::~JZNodeObjectFormat()
{

}

QString JZNodeObjectFormat::variantToString(const QVariant *v)
{
    int type = JZNodeType::variantType(*v);
    if(type == Type_list)
        return listToString(JZObjectCast<QVariantList>(*v));
    else if (type == Type_map)
        return mapToString(JZObjectCast<QVariantMap>(*v));
    else if (JZNodeType::isObject(type))
        return objectToString(toJZObject(*v));
    else    
        return JZNodeType::convertTo(Type_string, *v).toString();    
}

QString JZNodeObjectFormat::listToString(const QVariantList *list)
{
    QString context = "[";
    for (int i = 0; i < list->size(); i++)
    {
        context += variantToString(&list->at(i));
        if (i != list->size() - 1)
            context += ",";
    }
    context += "]";
    return context;         
}

QString JZNodeObjectFormat::mapToString(const QVariantMap *map)
{
    QString context = "{";
    auto it = map->begin();
    while (it != map->end())
    {
        QString name = "\"" + it.key() + "\"";
        QString value = variantToString(&it.value());
        context += name + ":" + value;

        it++;
        if(it != map->end())
            context += ",";
    }
    context += "}";
    return context;
}

QString JZNodeObjectFormat::objectToString(JZNodeObject *obj)
{    
    QString text = obj->className() + "{";

    auto func_def = obj->function("toString");
    if (func_def)
    {        
        QVariantList in, out;
        in << QVariant::fromValue(obj);;
        JZScriptInvoke(func_def->fullName(), in, out);
        text += out[0].toString();
    }
    else
    {
        auto params = obj->paramList();
        for (int i = 0; i < params.size(); i++)
        {
            QString name = "\"" + params[i] + "\"";
            QString value = variantToString(&obj->param(params[i]));
            text += name + ":" + value;
            if (i != params.size() - 1)
                text += ",";
        }
    }
    text += "}";
    return text;
}

QString JZNodeObjectFormat::format(JZNodeObject *obj)
{
    if (obj->type() == Type_list)
    {
        QVariantList *list = JZObjectCast<QVariantList>(obj);
        return listToString(list);
    }
    else if (obj->type() == Type_map)
    {
        QVariantMap *map = JZObjectCast<QVariantMap>(obj);
        return mapToString(map);
    }
    else
    {
        return objectToString(obj);
    }    
}
