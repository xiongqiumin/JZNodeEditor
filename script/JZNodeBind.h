#ifndef JZNODE_BIND_H_
#define JZNODE_BIND_H_

#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"
#include "JZEvent.h"

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

/// Backports of std::bool_constant and std::negation to accommodate older compilers
template <bool B> using bool_constant = std::integral_constant<bool, B>;
template <typename T> struct negation : bool_constant<!T::value> { };

/// Compile-time all/any/none of that check the boolean value of all template types
#if defined(__cpp_fold_expressions) && !(defined(_MSC_VER) && (_MSC_VER < 1916))
template <class... Ts> using all_of = bool_constant<(Ts::value && ...)>;
template <class... Ts> using any_of = bool_constant<(Ts::value || ...)>;
#elif !defined(_MSC_VER)
template <bool...> struct bools {};
template <class... Ts> using all_of = std::is_same<
    bools<Ts::value..., true>,
    bools<true, Ts::value...>>;
template <class... Ts> using any_of = negation<all_of<negation<Ts>...>>;
#else
// MSVC has trouble with the above, but supports std::conjunction, which we can use instead (albeit
// at a slight loss of compilation efficiency).
template <class... Ts> using all_of = std::conjunction<Ts...>;
template <class... Ts> using any_of = std::disjunction<Ts...>;
#endif
template <class... Ts> using none_of = negation<any_of<Ts...>>;
template <class T, template<class> class... Predicates> using satisfies_none_of = none_of<Predicates<T>...>;

template <typename T> using is_lambda = satisfies_none_of<remove_reference_t<T>,
        std::is_function, std::is_pointer, std::is_member_pointer>;

/// Strip the class from a method type
template <typename T> struct remove_class { };
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...)> { using type = R (A...); };
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) const> { using type = R (A...); };

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

template<class T> void *createClass(){ return new T(); }
template<class T> void destoryClass(void *ptr){ delete (T*)ptr; }
template<class T> void copyClass(void *src,void *dst){ *((T*)src) = *((T*)dst); }

void *createClassAssert();
void destoryClassAssert(void *);
void copyClassAssert(void *,void *);

template<class T>
T getValue(QVariant v,std::true_type)
{
    JZNodeObject *obj = toJZObject(v);
    return (T)obj->cobj;
}

template<class T>
T getValue(QVariant v,std::false_type)
{    
    if(v.type() == QVariant::UserType)
    {
        JZNodeObject *obj = toJZObject(v);
        T *cobj = (T*)(obj->cobj);
        return *cobj;
    }
    else
    {
        return v.value<T>();
    }
}

template<>
QVariant getValue<QVariant>(QVariant v,std::false_type);

template<class T>
remove_cvr_t<T> getValue(QVariant v)
{
    return getValue<remove_cvr_t<T>>(v,std::is_pointer<T>());
}

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
    list.push_back(typeid(typename std::remove_pointer<T>::type).name());
    if((sizeof ... (Args)) > 0 )
        getFunctionParam<type,Args...>(list);
}

template<class T>
QVariant getReturn(T value,bool isRef,std::true_type)
{
    JZNodeObjectPtr obj = JZObjectRefrence<T>(value);    
    if(!isRef)
        obj->cowner = true;
    return QVariant::fromValue(obj);
}

template<class T>
QVariant getReturn(T value,bool,std::false_type)
{
    if(std::is_class<T>())
    {
        JZNodeObjectPtr obj = JZObjectCreate<T>();
        *((T*)obj->cobj) = value;
        return QVariant::fromValue(obj);
    }
    else
    {
        return value;
    }
}

template<class T>
QVariant getReturn(T value,bool isRef)
{
    return getReturn<remove_cvr_t<T>>(value,isRef,std::is_pointer<T>());
}

template<>
QVariant getReturn(QVariant value,bool);

template<>
QVariant getReturn(QString value,bool);

template<typename Func,typename Return, typename... Args>
class CFunctionImpl : public CFunction
{    
public:
    CFunctionImpl(Func f)
        :func(f)
    {
        result = typeid(Return).name();
        getFunctionParam<int, Args...>(args);
        isRef = false;
    }

    virtual ~CFunctionImpl()
    {
    }    

    virtual void call(const QVariantList &in,QVariantList &out)
    {
        out.clear();
        dealCall(in,out,std::is_void<Return>());
    }

    void dealCall(const QVariantList &in,QVariantList&,std::true_type)
    {
        callFunction(in,make_index_sequence<sizeof...(Args)>());
    }

    void dealCall(const QVariantList &in,QVariantList &out,std::false_type)
    {
        Return ret = callFunction(in,make_index_sequence<sizeof...(Args)>());
        out.push_back(getReturn(ret,isRef));
    }    

    template<typename U = Return>
    typename std::enable_if<std::is_pointer<U>::value,void>::type setRefrence(bool flag)
    {
        isRef = flag;
    }

    template<typename U = Return>
    typename std::enable_if<!std::is_pointer<U>::value,void>::type setRefrence()
    {
    }

    template <size_t... idx>
    Return callFunction(const QVariantList &list,index_sequence<idx...>)
    {                
        return func((getValue<Args>(list[idx]))...);
    }

    Func func;
    bool isRef;
};

template <typename Func,typename Return, typename... Args>
CFunctionImpl<Func,Return,Args...> *createCFunction(Func func,Return (*)(Args...))
{
    return new CFunctionImpl<Func,Return,Args...>(func);
}

template<typename Return,typename... Args,typename... Extra>
void extra_check(Return (*)(Args...),Extra...)
{
    static_assert(!std::is_pointer<Return>::value || sizeof...(Extra) == 1,"if return point type, need set refrence type");
    static_assert(std::is_pointer<Return>::value || sizeof...(Extra) == 0,"if return no point type, don't set refrence type");
}

template <typename Func,typename... Extra>
CFunction *createFuncionImpl(Func func,Extra... extra)
{
    extra_check((function_signature_t<Func>*) nullptr,extra...);
    auto impl = createCFunction(func,(function_signature_t<Func>*) nullptr);
    impl->setRefrence(extra...);
    return impl;
}

template <typename Return, typename... Args,typename... Extra>
CFunction *createFuncion(Return (*f)(Args...),Extra... extra)
{
    auto func = [f](Args... args)->Return{ return f(args...);};
    return createFuncionImpl<decltype(func),Extra...>(func,extra...);
}

template <typename Func,typename... Extra>
CFunction *createFuncion(Func f,Extra... extra)
{
    return createFuncionImpl<decltype(f),Extra...>(f,extra...);
}

template <typename Return, typename Class, typename... Args,typename... Extra>
CFunction *createFuncion(Return (Class::*f)(Args...),Extra... extra)
{
    auto func = [f](Class *inst,Args... args)->Return{ return (inst->*f)(args...);};
    return createFuncionImpl<decltype(func),Extra...>(func,extra...);
}

template <typename Return, typename Class, typename... Args,typename... Extra>
CFunction *createFuncion(Return (Class::*f)(Args...) const,Extra... extra)
{
    auto func = [f](Class *inst,Args... args)->Return{ return (inst->*f)(args...);};    
    return createFuncionImpl<decltype(func),Extra...>(func,extra...);
}

//signle
template<class type>
void createEvent(JZEvent &)
{
    return;
}

template <class type,typename T,typename... Args>
void createEvent(JZEvent &event,const T &v,Args... args)
{
    event.params.push_back(getReturn(v,true));
    if((sizeof ... (Args)) > 0 )
        createEvent<type,Args...>(event,args...);
}

template<typename Class, typename... Args>
class CSingleImpl : public CSingle
{
public:
    virtual void connect(JZNodeObject *obj,int eventType)
    {
        Class *cobj = (Class*)obj->cobj;
        cobj->connect(cobj,single,[obj,eventType](Args... args){
           JZEvent event;
           event.sender = obj;
           event.eventType = eventType;
           event.params.push_back(QVariant::fromValue(event.sender));
           createEvent<int,Args...>(event,args...);
           JZObjectEvent(&event);
        });
    }

    void (Class::*single)(Args...);    
};


template<class Class>
class ClassBind
{
public:
    ClassBind(QString name,QString super = QString())
    {
        m_define.className = name;
        m_define.superName = super;
        m_define.isCObject = true;        
        initCreate(std::is_abstract<Class>());
        initCopy(std::is_copy_constructible<Class>());
        m_super = super;
    }

    void initCreate(std::false_type)
    {
        m_define.cMeta.create = &createClass<Class>;
        m_define.cMeta.destory = &destoryClass<Class>;
        m_define.cMeta.isAbstract = false;
    }

    void initCreate(std::true_type)
    {
        m_define.cMeta.create = createClassAssert;
        m_define.cMeta.destory = destoryClassAssert;
        m_define.cMeta.isAbstract = true;
    }

    void initCopy(std::true_type)
    {
        m_define.cMeta.copy = &copyClass<Class>;
        m_define.cMeta.isCopyable = true;
    }

    void initCopy(std::false_type)
    {
        m_define.cMeta.copy = &copyClassAssert;
        m_define.cMeta.isCopyable = false;
    }

    template<typename Return, typename... Args,typename... Extra>
    void def(QString name,bool isflow,Return (Class::*f)(Args...),Extra ...extra)
    {
        auto impl = createFuncion(f,extra...);
        registFunction(name,isflow,impl);
    }

    template<typename Return, typename... Args,typename... Extra>
    void def(QString name,bool isflow,Return (Class::*f)(Args...) const,Extra ...extra)
    {
        auto impl = createFuncion(f,extra...);
        registFunction(name,isflow,impl);
    }

    template <typename Func,typename... Extra>
    void def(QString name,bool isflow,Func f,Extra ...extra)
    {
        auto impl = createFuncion(f,extra...);
        registFunction(name,isflow,impl);
    }       

    template<typename... Args>
    void defSingle(QString name,int eventType,void (Class::*f)(Args...))
    {
        registFunction(name,true,createFuncion(f));

        SingleDefine single;
        single.name = name;
        single.eventType = eventType;

        auto *impl = new CSingleImpl<Class,Args...>();        
        impl->single = f;
        single.isCSingle = true;
        single.csingle = impl;        
        m_define.singles.push_back(single);
    }

    void regist(int typeId = -1)
    {
        m_define.id = typeId;
        JZNodeObjectManager::instance()->registCClass(m_define,typeid(Class).name());
        for(int i = 0; i < m_functions.size(); i++)
        {
            auto &f = m_functions[i];
            JZNodeFunctionManager::instance()->registCFunction(m_define.className + "." + f.name,f.isFlow,f.func);
        }
        for(int i = 0; i < m_define.singles.size(); i++)
        {
            JZNodeFunctionManager::instance()->registCSingle(m_define.singles[i].csingle);
        }
        declareCClass(std::is_base_of<QObject,Class>());
    }

protected:
    struct FunctionInfo{
        QString name;
        bool isFlow;
        CFunction *func;
    };       

    void registFunction(QString name,bool isflow,CFunction *func)
    {
        FunctionInfo f;
        f.name = name;
        f.isFlow = isflow;
        f.func = func;
        m_define.functions.push_back(name);
        m_functions.push_back(f);
    }

    void declareCClass(std::true_type)
    {
        int id = JZNodeObjectManager::instance()->getClassIdByTypeid(typeid(Class).name());
        QString className = Class::staticMetaObject.className();
        JZNodeObjectManager::instance()->declareCClass(className,id);
    }

    void declareCClass(std::false_type)
    {

    }

    QString m_super;
    JZNodeObjectDefine m_define;
    QList<FunctionInfo> m_functions;
};


}

#endif
