#include <functional>
#include <stdexcept>
#include "JZContainer.h"
#include "JZNodeEngine.h"

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
    if (!flag)
    {
        QString error = QString::asprintf("not contains");
        throw std::runtime_error(qPrintable(error));
    }
}

void listForeach(JZNodeObject *obj, std::function<bool(int,QVariant)> vistor)
{
    auto env = g_engine->environment();

    QString list_type = obj->className();
    JZNodeObjectPointer holder(obj, false);
    
    QVariantList in, out;
    in << QVariant::fromValue(holder);

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
    QString it_type = JZNodeType::mapIteratorType(map_type);
    int it_ptr_type = env->nameToType(JZNodeType::pointerType(it_type));

    JZNodeObjectPointer holder(obj, false);

    QVariant it, it_end;
    QVariantList in, out;
    in << QVariant::fromValue(holder);

    JZScriptInvoke(map_type + "::beign", in, out);
    it = out[0];

    JZScriptInvoke(map_type + "::end", in, out);
    it_end = out[0];
    
    while (it != it_end)
    {
        QVariant it_ptr = JZConvertVariant(it,it_ptr_type);
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

//JZContainerManager
JZContainerManager *JZContainerManager::instance()
{
    static JZContainerManager inst;
    return &inst;
}

JZContainerManager::JZContainerManager()
{
    addListFunction("set", true, { "int", "arg" });
    addListFunction("get", false, { "int" }, {"arg"});
    addListFunction("size", false, {}, { "int" });
    addListFunction("clear", true, {});

    addListFunction("insert", true, { "int", "arg" });
    addListFunction("push_back", true, { "arg" });
    addListFunction("pop_back", true, { });
    addListFunction("push_front", true, { "arg" });
    addListFunction("pop_front", true, {});
    addListFunction("removeAt", true, { "int" });
    
    addListFunction("mid", false, { "int", "int" }, { "QList<arg>" });
    addListFunction("append", true, { "QList<arg>*" });
    addListFunction("resize", true, { "int" } );
    addListFunction("swap", true, { "int","int" })
        ;
    addListFunction("indexOf", false, { "arg", "int" }, { "int" });;
    addListFunction("lastIndexOf", true, { "arg", "int" }, { "int" });
    addListFunction("removeOne", true, { "arg"});
    addListFunction("removeAll", true, { "arg" });
    addListFunction("contains", false, { "arg" });

}

JZContainerManager::~JZContainerManager()
{
}

void JZContainerManager::addListFunction(QString function, bool isFlow, QStringList input, QString output)
{
    JZFunctionDefine def;
    def.setFullName("QList<arg>::" + function);
    def.isFlowFunction = isFlow;
    def.paramIn << JZParamDefine("this","QList<arg>*");
    for (int i = 0; i < input.size(); i++)
    {
        QString input_name = "input" + QString::number(i);
        def.paramIn << JZParamDefine(input_name, input[i]);
    }

    if (output.isEmpty())
    {
        def.paramOut << JZParamDefine("output", output);
    }

    m_functions.insert(def.fullName(), def);
}

void JZContainerManager::addMapFunction(QString function, bool isFlow, QStringList input, QString output)
{

}

JZFunctionDefine *JZContainerManager::function(QString name)
{
    auto it = m_functions.find(name);
    if (it == m_functions.end())
        return nullptr;

    return &it.value();
}

QStringList JZContainerManager::functionList()
{
    return m_functions.keys();
}

void JZContainerManager::regist(JZScriptEnvironment* env)
{

}