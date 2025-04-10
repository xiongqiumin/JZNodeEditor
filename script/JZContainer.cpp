#include <functional>
#include <stdexcept>
#include "JZContainer.h"
#include "JZNodeEngine.h"

QString listType(QString value)
{
    return "QList<" + value + ">";
}

QString listIteratorType(QString list_type)
{
    return list_type + "::iterator";
}

QString mapType(QString key, QString value)
{
    return "QMap<"  + key + "," + value + ">";
}

QString mapIteratorType(QString map_type)
{
    return map_type + "::iterator";
}

void checkEmpty(int size)
{
    if (size == 0)
    {
        QString error = QString::asprintf("is empty");
        throw std::runtime_error(qPrintable(error));
    }
}

void checkSize(int index, int size)
{
    if (index < 0 || index >= size)
    {
        QString error = QString::asprintf("index %d out of range %d", index, size);
        throw std::runtime_error(qPrintable(error));
    }
}

void checkContains(bool flag)
{
    if (flag)
    {
        QString error = QString::asprintf("not contains");
        throw std::runtime_error(qPrintable(error));
    }
}

void listForeach(JZNodeObject *obj, std::function<bool(int,QVariant)> vistor)
{
    auto env = g_engine->environment();

    QString list_type = obj->className();
    JZNodeObjectPointer list_ptr = JZNodeObjectPointer::fromObject(obj, true);
    
    QVariantList in, out;
    in << QVariant::fromValue(list_ptr);

    JZScriptInvoke(list_type + "::size", in, out);
    int list_size = out[0].toInt();

    int index = 0;
    while (index < list_size)
    {      
        JZScriptInvoke(list_type + "::value", in, out);
        QVariant value = out[0];
        if (!vistor(index, value))
            return;

        index++;
    }
}

void mapForeach(JZNodeObject *obj, std::function<bool(QVariant, QVariant)> vistor)
{
    auto env = g_engine->environment();

    QString map_type = obj->className();
    QString it_type = mapIteratorType(map_type);
    int it_ptr_type = env->nameToType(JZNodeType::pointerType(it_type));

    JZNodeObjectPointer map_ptr = JZNodeObjectPointer::fromObject(obj, true);

    QVariant it, it_end;
    QVariantList in, out;
    in << QVariant::fromValue(map_ptr);

    JZScriptInvoke(map_type + "::beign", in, out);
    it = out[0];

    JZScriptInvoke(map_type + "::end", in, out);
    it_end = out[0];
    
    while (it != it_end)
    {
        QVariant it_ptr = JZScriptConvert(it,it_ptr_type);
        QVariantList it_call_in;
        it_call_in << it_ptr;

        QVariant key, value;
        JZScriptInvoke(it_type + "::key", it_call_in, out);
        key = out[0];

        JZScriptInvoke(it_type + "::value", it_call_in, out);
        value = out[0];

        if (!vistor(key, value))
            return;

        JZScriptInvoke(it_type + "::next", in, out);        
    }
}