#ifndef JZNODE_FUNCTION_DEFINE_H_
#define JZNODE_FUNCTION_DEFINE_H_

#include "JZNode.h"

class JZNodeScript;

class FunctionParam
{
public:
    QString name;

    QStringList input;
    QString output;
};

class CFunction
{
public:
    CFunction();
    virtual ~CFunction();
    virtual void call(const QVariantList &in,QVariantList &out) = 0;

    QStringList args;
    QString result;
};

template<size_t ...> struct index_sequence  { };
template<size_t N, size_t ...S> struct make_index_sequence_impl : make_index_sequence_impl <N - 1, N - 1, S...> { };
template<size_t ...S> struct make_index_sequence_impl <0, S...> { using type = index_sequence<S...>; };
template<size_t N> using make_index_sequence = typename make_index_sequence_impl<N>::type;

template<typename Return, typename... Args>
class CFunctionImpl : public CFunction
{
    using Func = Return (*)(Args...);
public:
    virtual void call(const QVariantList &in,QVariantList &out)
    {
        if(typeid(Return) == typeid(void))
            callFunction(in,make_index_sequence<sizeof...(Args)>());
        else
        {
            Return ret = callFunction(in,make_index_sequence<sizeof...(Args)>());
            out.push_back(ret);
        }
    }

    template <size_t... idx>
    Return callFunction(const QVariantList &list,index_sequence<idx...> seq)
    {
        return func((list[idx].value<Args>())...);
    }

    Func func;
};

template<typename Return, typename Class,typename... Args>
class CClassFunctionImpl : public CFunction
{
    using Func = Return (Class::*)(Args...);
public:
    virtual void call(const QVariantList &in,QVariantList &out)
    {
        if(typeid(Return) == typeid(void))
            callFunction(in,make_index_sequence<sizeof...(Args)>());
        else
        {
            Return ret = callFunction(in,make_index_sequence<sizeof...(Args)>());
            out.push_back(ret);
        }
    }

    template <size_t... idx>
    Return callFunction(const QVariantList &list,index_sequence<idx...> seq)
    {
        Class *inst = list[0].value<Class*>();
        return (inst->*func)((list[idx+1].value<Args>())...);
    }

    Func func;
};

//这里加一个空模板函数是为了编译可以通过，否则编译期间调用printAmt<int>(int&)就会找不到可匹配的函数
//模板参数第一个类型实际上是用不到的，但是这里必须要加上，否则就是调用printAmt<>(int&)，模板实参为空，但是模板形参列表是不能为空的
template<class type>
void getFunctionParam(QStringList &list)
{
    return;
}

template <class type,typename T,typename... Args>
void getFunctionParam(QStringList &list)
{
    list.push_back(typeid(T).name());
    if((sizeof ... (Args)) > 0 )
        getFunctionParam<type,Args...>(list);
}

template <typename Return, typename... Args>
CFunctionImpl<Return,Args...> *createFuncion(Return (*f)(Args...))
{
    CFunctionImpl<Return,Args...> *impl = new CFunctionImpl<Return,Args...>();
    impl->func = f;
    impl->result = typeid(Return).name();
    getFunctionParam<int, Args...>(impl->args);
    return impl;
}

template <typename Return, typename Class, typename... Args>
CClassFunctionImpl<Return,Class,Args...> *createClassFuncion(Return (Class::*f)(Args...))
{
    CClassFunctionImpl<Return,Class,Args...> *impl = new CClassFunctionImpl<Return,Class,Args...>();
    impl->func = f;
    impl->result = typeid(Return).name();
    getFunctionParam<int, Args...>(impl->args);
    return impl;
}

class FunctionDefine
{
public:
    FunctionDefine();    
    
    bool isCFunction;
    QString name;
    QList<JZNodePin> paramIn;
    QList<JZNodePin> paramOut;    
    
    //for node
    int addr;
    JZNodeScript *script;

    //for c    
    CFunction *cfunc;    
};

#endif
