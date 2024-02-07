#include "JZNodeObjectParser.h"
#include <memory>
#include <JZRegExpHelp.h>

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
            return false;
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
        return false;

    auto meta = JZNodeObjectManager::instance()->meta(type);
    if (!meta)
        return false;

    JZNodeObject *obj = JZNodeObjectManager::instance()->create(meta->id);
    QScopedPointer<JZNodeObject> ptr(obj);
    if (meta->function("fromString"))
    {
        
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

            if (!JZNodeType::canConvert(param_def->dataType, it.value()))
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

JZNodeObject *JZNodeObjectParser::parse(const QString &text)
{             
    iniContext(text);

    JZNodeObject *obj = nullptr;
    QChar c = nextChar();
    if (c == '{')
    {
        QVariantMap *map = readMap();
        if (!map)
            return false;        
        
        obj = JZNodeObjectManager::instance()->createCClassRefrence(Type_map, map, true);        
    }
    else if (c == '[')
    {
        QVariantList *list = readList();
        if (!list)
            return false;
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

QString JZNodeObjectFormat::format(JZNodeObject *obj)
{
    return QString();
}