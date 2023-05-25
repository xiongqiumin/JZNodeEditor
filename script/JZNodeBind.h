#ifndef JZNODE_BIND_H_
#define JZNODE_BIND_H_

#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"

namespace jzbind
{
//copy from pybind11
template<size_t ...> struct index_sequence  { };
template<size_t N, size_t ...S> struct make_index_sequence_impl : make_index_sequence_impl <N - 1, N - 1, S...> { };
template<size_t ...S> struct make_index_sequence_impl <0, S...> { using type = index_sequence<S...>; };
template<size_t N> using make_index_sequence = typename make_index_sequence_impl<N>::type;

template <bool B, typename T = void> using enable_if_t = typename std::enable_if<B, T>::type;
template <bool B, typename T, typename F> using conditional_t = typename std::conditional<B, T, F>::type;
template <typename T> using remove_cv_t = typename std::remove_cv<T>::type;
template <typename T> using remove_reference_t = typename std::remove_reference<T>::type;
template <typename T> using remove_cvr_t = remove_cv_t<remove_reference_t<T>>;

/// Strip the class from a method type
template <typename T> struct remove_class { };
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...)> { using type = R (A...); };
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) const> { using type = R (A...); };

template<class T> void *createClass(){ return new T(); }
template<class T> void destoryClass(void *ptr){ delete (T*)ptr; }
template<class T> void copyClass(void *src,void *dst){ *((T*)src) = *((T*)dst); }

template<class T>
T getValue(QVariant v,std::true_type)
{
    JZNodeObjectPtr obj = v.value<JZNodeObjectPtr>();
    return (T)obj->cobj;
}

template<class T>
T getValue(QVariant v,std::false_type)
{    
    if(std::is_class<T>())
    {
        JZNodeObjectPtr obj = v.value<JZNodeObjectPtr>();
        T *cobj = (T*)(obj->cobj);
        return *cobj;
    }
    else
    {
        return v.value<T>();
    }
}

template<class T>
remove_cvr_t<T> getValue(QVariant v)
{
    return getValue<remove_cvr_t<T>>(v,std::is_pointer<T>());
}

template<class T>
QVariant getReturn(T value)
{
    if(std::is_class<T>())
    {        
        JZNodeObjectPtr obj = JZNodeObjectManager::instance()->createCClass(typeid(T).name());
        *((T*)obj->cobj) = value;
        return QVariant::fromValue(obj);
    }
    else
    {
        return value;
    }
}

template <typename F> struct strip_function_object {
    // If you are encountering an
    // 'error: name followed by "::" must be a class or namespace name'
    // with the Intel compiler and a noexcept function here,
    // try to use noexcept(true) instead of plain noexcept.
    using type = typename remove_class<decltype(&F::operator())>::type;
};

// Extracts the function signature from a function, function pointer or lambda.
template <typename Function, typename F = remove_reference_t<Function>>
using function_signature_t = conditional_t<
    std::is_function<F>::value,
    F,
    typename conditional_t<
        std::is_pointer<F>::value || std::is_member_pointer<F>::value,
        std::remove_pointer<F>,
        strip_function_object<F>
    >::type
>;

template<typename Func,typename Return, typename... Args>
class CFunctionImpl : public CFunction
{    
public:
    CFunctionImpl(Func f)
        :func(f)
    {

    }

    virtual void call(const QVariantList &in,QVariantList &out)
    {
        out.clear();
        call(in,out,std::is_void<Return>());
    }

    void call(const QVariantList &in,QVariantList&,std::true_type)
    {
        callFunction(in,make_index_sequence<sizeof...(Args)>());
    }

    void call(const QVariantList &in,QVariantList &out,std::false_type)
    {
        Return ret = callFunction(in,make_index_sequence<sizeof...(Args)>());
        out.push_back(getReturn(ret));
    }

    template <size_t... idx>
    Return callFunction(const QVariantList &list,index_sequence<idx...>)
    {
        return func((getValue<Args>(list[idx]))...);
    }

    Func func;
};


//这里加一个空模板函数是为了编译可以通过，否则编译期间调用printAmt<int>(int&)就会找不到可匹配的函数
//模板参数第一个类型实际上是用不到的，但是这里必须要加上，否则就是调用printAmt<>(int&)，模板实参为空，但是模板形参列表是不能为空的
template<class type>
void getFunctionParam(QStringList &)
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

template <typename Func,typename Return, typename... Args>
CFunction *createLambdaFuncion(Func &&func,Return (*)(Args...))
{
    CFunction *impl = new CFunctionImpl<Func,Return,Args...>(func);
    impl->result = typeid(Return).name();
    getFunctionParam<int, Args...>(impl->args);
    return impl;
}

template <typename Return, typename... Args>
CFunction *createFuncion(Return (*f)(Args...))
{
    auto func = [f](Args... args)->Return{ return f(args...);};
    auto *impl = new CFunctionImpl<decltype(func),Return,Args...>(func);
    impl->result = typeid(Return).name();
    getFunctionParam<int, Args...>(impl->args);
    return impl;
}

template <typename Func>
CFunction *createFuncion(Func &&f)
{
    return createLambdaFuncion(f,(function_signature_t<Func>*) nullptr);
}

template <typename Return, typename Class, typename... Args>
CFunction *createFuncion(Return (Class::*f)(Args...))
{
    auto func = [f](Class *inst,Args... args)->Return{ return (inst->*f)(args...);};
    auto *impl = new CFunctionImpl<decltype(func),Return,Class*,Args...>(func);
    impl->result = typeid(Return).name();    
    getFunctionParam<int, Class*,Args...>(impl->args);
    return impl;
}

template <typename Return, typename Class, typename... Args>
CFunction *createFuncion(Return (Class::*f)(Args...) const)
{
    auto func = [f](Class *inst,Args... args)->Return{ return (inst->*f)(args...);};
    auto *impl = new CFunctionImpl<decltype(func),Return,Class*,Args...>(func);
    impl->result = typeid(Return).name();    
    getFunctionParam<int, Class*,Args...>(impl->args);
    return impl;
}

template<class Class>
class ClassBind
{
public:
    ClassBind(QString name,QString super = QString())
    {
        m_define.className = name;
        m_define.isCObject = true;
        m_define.cMeta.create = &createClass<Class>;
        m_define.cMeta.destory = &destoryClass<Class>;
        m_define.cMeta.copy = &copyClass<Class>;
        m_super = super;
    }

    template<typename Return, typename... Args>
    void def(QString name,Return (Class::*f)(Args...))
    {
        auto impl = createFuncion(f);
        registFunction(name,impl);
    }

    template<typename Return, typename... Args>
    void def(QString name,Return (Class::*f)(Args...) const)
    {
        auto impl = createFuncion(f);
        registFunction(name,impl);
    }

    template <typename Func>
    void def(QString name,Func &&f)
    {
        auto impl = createFuncion(f);
        registFunction(name,impl);
    }       

    void regist()
    {
        JZNodeObjectManager::instance()->registCClass(m_define,typeid(Class).name(),m_super);
    }

protected:
    void registFunction(QString name,CFunction *func)
    {
        JZNodeFunctionManager::instance()->registCFunction(m_define.className + "." + name,func);
    }

    QString m_super;
    JZNodeObjectDefine m_define;
};


}

#endif
