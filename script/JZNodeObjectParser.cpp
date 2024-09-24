#include <memory>
#include <JZRegExpHelp.h>
#include "JZNodeEngine.h"
#include "JZNodeObjectParser.h"

JZNodeObjectParser::Token::Token()
{
    index = -1;
}

JZNodeObjectParser::JZNodeObjectParser()
{
    m_currentIndex = 0;
    m_gapList << ',' << ':' << '[' << ']' << '{' << '}';
}

JZNodeObjectParser::~JZNodeObjectParser() 
{        
}

bool JZNodeObjectParser::iniContext(const QString &text)
{
    m_content = text;
    m_currentIndex = 0;
    m_error.clear();
    m_tokenList.clear();

    QString pre;
    int pre_idx = -1;

    auto pushPre = [this,&pre,&pre_idx] 
    {
        if(pre.size() != 0)
        {
            Token tk;
            tk.index = pre_idx;
            tk.word = pre;
            m_tokenList << tk;

            pre.clear();
            pre_idx = -1;
        }
    };

    for(int i = 0; i < m_content.size(); i++)
    {
        QChar c = m_content[i];
        if(m_gapList.contains(c) || c.isSpace())
        {
            pushPre();
            if(!c.isSpace())
            {
                Token tk;
                tk.index = i;
                tk.word = QString(c);
                m_tokenList << tk;
            }
        }
        else if(c == '"')
        {
            pushPre();

            bool find = true;
            i++;

            QString str;
            str.push_back(c);
            while (i < m_content.size())
            {
                c = m_content[i];
                str.push_back(c);

                if (c == '\\')
                {
                }
                else if (c == '"')
                {
                    find = true;
                    break;
                }
                i++;
            }

            if(!find)
            {
                m_error = "string not end";
                return false;
            }

            Token tk;
            tk.index = i;
            tk.word = str;
            m_tokenList << tk;
        }
        else
        {
            if(pre.isEmpty())
                pre_idx = i;

            pre.push_back(c);
            if(c == '<')
            {   
                i++;

                bool find = false;
                int bkt_num = 1;
                while(i < m_content.size())
                {
                    pre.push_back(m_content[i]);
                    if(m_content[i] == '<')
                        bkt_num++;
                    if(m_content[i] == '>')
                    {
                        bkt_num--;
                        if(bkt_num == 0)
                        {
                            find = true;
                            break;
                        }
                    }
                    i++;
                }

                if(!find)
                {
                    m_error = "> not end";
                    return false;
                }
            }
        }
    }
    pushPre();

    return true;
}

void JZNodeObjectParser::getRowCol(int index,int &row,int &col)
{
    row = 0;
    col = 0;
    for(int i = 0; i < index; i++)
    {   
        col++;
        if(m_content[i] == '\n')
        {
            row++;
            col = 0;
        }
    }
}

void JZNodeObjectParser::makeError(const QString &error)
{
    int line,col;
    getRowCol(m_currentIndex,line,col);
    m_error = "row: " + QString::number(line + 1) + ",col: " + QString::number(col + 1) + " " + error; 
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

JZNodeObjectParser::Token JZNodeObjectParser::nextToken()
{
    if(m_currentIndex  < m_tokenList.size())
        return m_tokenList[m_currentIndex];
    else
    {
        return Token();
    }
}

JZNodeObjectParser::Token JZNodeObjectParser::readToken()
{
    if(m_currentIndex < m_tokenList.size())
    {
        int index = m_currentIndex;
        m_currentIndex++;
        return m_tokenList[index];
    }
    else
    {
        return Token();
    }
}

void JZNodeObjectParser::pushToken()
{
    Q_ASSERT(m_currentIndex > 0);
    m_currentIndex--;
}

JZList *JZNodeObjectParser::readList(QString valueType,QString gap)
{    
    Token c = readToken();
    Q_ASSERT(c.word == gap);
    gap = (gap == "[")? "]":"}";

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

        c = readToken();
        if (c.word == gap)
            break;
       
        if (c.word != ",")
        {
            makeExpectError(",",c.word);
            return nullptr;
        }
    }

    return ptr.take();    
}

JZMap *JZNodeObjectParser::readMap(QString keyType,QString valueType)
{
    Token c = readToken();
    Q_ASSERT(c.word == "{");

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
        c = readToken();
        if (c.word != ":")
        {
            makeExpectError(":",c.word);
            return nullptr;        
        }

        QVariant value = readVariable();
        if(!checkVariable(value,value_type))
            return nullptr;
        value = JZNodeType::convertTo(key_type,value);

        JZMap::Key map_key;
        map_key.v = key;
        ptr->map.insert(map_key, value);

        c = readToken();
        if (c.word == "}")
            break;
        if (c.word != ",")
        {
            makeExpectError(",",c.word);
            return nullptr;
        }
    }

    return ptr.take();    
}

JZNodeObject *JZNodeObjectParser::readObject()
{    
    auto inst = g_engine->environment()->objectManager();
    Token tk = nextToken();
    QString type = tk.word;

    if (type == "{")
    {
        auto map = readMap("string","any");
        if(map)
            return inst->createRefrence(map->type(),map,true);
        return nullptr;
    }
    else if(type == "[")
    {
        auto list = readList("any","[");
        if(list)
            return inst->createRefrence(list->type(),list,true);
        return nullptr;
    }
    
    auto meta = inst->meta(type);
    if (!meta)
    {
        makeError("no object " + type);
        return nullptr;
    }

    readToken();
    if(nextToken().word != "{")
    {
        makeExpectError("{", nextToken().word);
        return nullptr;
    }

    QString list_pre = "QList<";
    QString map_pre = "QMap<";
    if(type.startsWith(list_pre))
    {
        int end_idx = type.lastIndexOf(">");
        QString value_type = type.mid(list_pre.size(),end_idx - list_pre.size());
        auto list = readList(value_type,"{");
        if(list)
            return inst->createRefrence(list->type(),list,true);
        return nullptr;
    }
    else if(type.startsWith(map_pre))
    {
        int end_idx = type.lastIndexOf(">");
        QStringList type_list = type.mid(map_pre.size(),end_idx - map_pre.size()).split(",");
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
        JZNodeObject *obj = inst->create(meta->id);
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
    Token tk = readToken();

    QChar c = tk.word[0];
    JZNodeObject *obj = nullptr;
    if (c.isDigit() || c == '-')
    {
        QString text = tk.word;
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
        return tk.word;
    }
    else
    {
        pushToken();
        obj = readObject();
        if (!obj)
            return QVariant();
        return QVariant::fromValue(JZNodeObjectPtr(obj,true));
    }    
   
}

bool JZNodeObjectParser::checkIsEnd()
{    
    return (m_currentIndex == m_tokenList.size());
}

bool JZNodeObjectParser::readBkt(QString &context)
{
    if (readToken().word != "{")
        return false;

    int start = nextToken().index;
    int bkt_num = 1;
    while (m_currentIndex < m_tokenList.size())
    {
        Token c = m_tokenList[m_currentIndex++];

        if (c.word == "{")
            bkt_num++;
        else if (c.word == "}")
        {
            bkt_num--;
            if (bkt_num == 0)
            {
                context = m_content.mid(start,c.index - start);
                return true;
            }
        }
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
        m_error = "no expect char '" + nextToken().word + "'";
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

bool JZNodeObjectParser::parseVariantList(const QString &format, const QString &text,QVariantList &result)
{
    QVariantList ret;
    iniContext(text);

    for (int i = 0; i < format.size(); i++)
    {
        QString c = format[i];
        Token tk = readToken();
        QString word = tk.word;
        
        if (c == 's')
        {
            if (!JZRegExpHelp::isString(word))
            {
                makeExpectError("string", word);
                return false;
            }
            ret.push_back(word);
        }
        else if (c == 'i')
        {
            if (!JZRegExpHelp::isInt(word) && !JZRegExpHelp::isHex(word))
            {
                makeExpectError("int", word);
                return false;
            }
            ret.push_back(word.toInt());
        }
        else if (c == 'd')
        {
            if (!JZRegExpHelp::isFloat(word))
            {
                makeExpectError("float", word);
                return false;
            }
            ret.push_back(word.toDouble());
        }
        else
        {
            if (c != word)
            {
                makeExpectError(c, word);
                return false;
            }
        }
    }
    result = ret;
    return true;
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
    if(list->type() != "QMap<string,any>")
        context = list->type() + "{";
    else
        context = "[";
    for (int i = 0; i < list->list.size(); i++)
    {
        context += variantToString(&list->list.at(i));
        if (i != list->list.size() - 1)
            context += ",";
    }
    if(list->type() != "QMap<string,any>")
        context += "}";
    else
        context += "]";

    return context;         
}

QString JZNodeObjectFormat::mapToString(const JZMap *map)
{
    QString context;
    if(map->type() != "QMap<string,any>")
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
