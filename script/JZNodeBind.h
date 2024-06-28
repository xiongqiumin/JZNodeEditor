#ifndef JZNODE_BIND_H_
#define JZNODE_BIND_H_

#include <type_traits>
#include <QMetaEnum>
#include <QSet>

#include <QPaintEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMouseEvent>

#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"
#include "JZEvent.h"

extern void JZScriptOnSlot(JZNodeObject *sender,const QString &function,const QVariantList &in, QVariantList &out);
extern bool JZScriptInvoke(const QString &function,const QVariantList &in, QVariantList &out);

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

template<class T>
constexpr bool is_enum_or_qenum_cond()
{
    return std::is_enum<T>() || QtPrivate::IsQEnumHelper<T>::Value;
}

template<class T>
using is_enum_or_qenum = bool_constant<is_enum_or_qenum_cond<T>()>;


template<class T> void *createClass(){ return new T(); }
template<class T> void destoryClass(void *ptr){ delete (T*)ptr; }
template<class T> void copyClass(void *src,void *dst){ *((T*)src) = *((T*)dst); }

void *createClassAssert();
void destoryClassAssert(void *);
void copyClassAssert(void *,void *);

// from QVariant
template<class T>
T fromVariantEnum(const QVariant &v, std::true_type)
{
    Q_ASSERT(v.type() == QVariant::Int);
    return (T)v.toInt();
}

template<class T>
T fromVariantEnum(const QVariant &v, std::false_type)
{
    if(v.type() == QVariant::UserType)
    {
        JZNodeObject *obj = toJZObject(v);
        T *cobj = (T*)(obj->cobj());
        return *cobj;
    }
    else
    {
        return v.value<T>();
    }
}

template<class T>
T fromVariant(const QVariant &v, std::true_type)
{
    JZNodeObject *obj = toJZObject(v);
    if (obj)
        return (T)obj->cobj();
    else
        return nullptr;
}

template<class T>
T fromVariant(const QVariant &v, std::false_type)
{
    return fromVariantEnum<T>(v, is_enum_or_qenum<T>());
}

template<>
QString fromVariant<QString>(const QVariant &v, std::false_type);

//为了调用QString 成员函数，比如 QString.size();
template<>
QString* fromVariant<QString*>(const QVariant &v, std::true_type);

template<class T>
remove_cvr_t<T> fromVariant(const QVariant &v)
{
    static_assert(!std::is_same<QVariant, remove_cvr_t<T>>::value, "use JZNodeVariantAny");
    return fromVariant<remove_cvr_t<T>>(v, std::is_pointer<T>());
}

//to QVariant
template<class T>
QVariant toVariantEnum(T value, std::true_type)
{
    return (int)value;
}

template<class T>
QVariant toVariantEnum(T value, std::false_type)
{
    return value;
}

template<class T>
QVariant toVariantClass(T value, std::true_type)
{
    JZNodeObject* obj = JZObjectCreate<T>();
    *((T*)obj->cobj()) = value;
    return QVariant::fromValue(obj);
}

template<class T>
QVariant toVariantClass(T value, std::false_type)
{
    return toVariantEnum(value, is_enum_or_qenum<T>());
}

template<class T>
QVariant toVariantPointer(T value, std::true_type)
{
    static_assert(std::is_class<std::remove_pointer_t<T>>(),"only support class pointer");
    JZNodeObject* obj = JZObjectRefrence<T>(value, false);
    return QVariant::fromValue(obj);
}

template<class T>
QVariant toVariantPointer(T value, std::false_type)
{
    return toVariantClass(value,std::is_class<T>());
}

template<class T>
QVariant toVariant(T value)
{
    static_assert(!std::is_same<QVariant, T>::value, "use JZNodeVariantAny");
    return toVariantPointer<remove_cvr_t<T>>(value,std::is_pointer<T>());
}

template<>
QVariant toVariant(JZNodeVariantAny value);

template<>
QVariant toVariant(QString value);

template <class type>
void toVariantList(QVariantList &list)
{
}

template <class type,typename T,typename... Args>
void toVariantList(QVariantList &list,T t,Args... args)
{
    list.push_back(toVariant(t));
    toVariantList<type,Args...>(list,args...);
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
    static_assert(!std::is_same<QVariant, T>::value, "use JZNodeVariantAny");
    list.push_back(typeid(typename std::remove_pointer<T>::type).name());
    getFunctionParam<type,Args...>(list);
}

template<typename Func,typename Return, typename... Args>
class CFunctionImpl : public CFunction
{    
public:
    CFunctionImpl(Func f)
        :func(f)
    {
        result = typeid(std::remove_pointer_t<Return>).name();
        getFunctionParam<int, Args...>(args);
        isRef = false;
        isPointer = false;
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
        auto v = toVariant(ret);
        if(isPointer && !isRef)
        {
            auto obj = toJZObject(v);
            obj->setCOwner(true);
        }
        out.push_back(v);
    }    

    template<typename U = Return>
    typename std::enable_if<std::is_pointer<U>::value,void>::type setRefrence(bool flag)
    {
        isRef = flag;
        isPointer = true;
    }

    template<typename U = Return>
    typename std::enable_if<!std::is_pointer<U>::value,void>::type setRefrence()
    {
    }

    template <size_t... idx>
    Return callFunction(const QVariantList &list,index_sequence<idx...>)
    {                
        return func((fromVariant<Args>(list[idx]))...);
    }

    Func func;
    bool isRef; //isRef 表示是否引用, false时, 才会自动管理
    bool isPointer;
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
QSharedPointer<CFunction> createFuncionImpl(Func func,Extra... extra)
{
    extra_check((function_signature_t<Func>*) nullptr,extra...);
    auto impl = createCFunction(func,(function_signature_t<Func>*) nullptr);
    impl->setRefrence(extra...);
    return QSharedPointer<CFunction>(impl);
}

template <typename Return, typename... Args,typename... Extra>
QSharedPointer<CFunction> createFuncion(Return (*f)(Args...),Extra... extra)
{
    auto func = [f](Args... args)->Return{ return f(args...);};
    return createFuncionImpl<decltype(func),Extra...>(func,extra...);
}

template <typename Func,typename... Extra>
QSharedPointer<CFunction> createFuncion(Func f,Extra... extra)
{
    return createFuncionImpl<decltype(f),Extra...>(f,extra...);
}

template <typename Return, typename Class, typename... Args,typename... Extra>
QSharedPointer<CFunction> createFuncion(Return (Class::*f)(Args...),Extra... extra)
{
    auto func = [f](Class *inst,Args... args)->Return{ return (inst->*f)(args...);};
    return createFuncionImpl<decltype(func),Extra...>(func,extra...);
}

template <typename Return, typename Class, typename... Args,typename... Extra>
QSharedPointer<CFunction> createFuncion(Return (Class::*f)(Args...) const,Extra... extra)
{
    auto func = [f](Class *inst,Args... args)->Return{ return (inst->*f)(args...);};    
    return createFuncionImpl<decltype(func),Extra...>(func,extra...);
}

#if __cplusplus >= 201103L
template <typename Return, typename Class, typename... Args, typename... Extra>
QSharedPointer<CFunction> createFuncion(Return(Class::*f)(Args...) noexcept, Extra... extra)
{
    auto func = [f](Class *inst, Args... args)->Return { return (inst->*f)(args...); };
    return createFuncionImpl<decltype(func), Extra...>(func, extra...);
}

template <typename Return, typename Class, typename... Args, typename... Extra>
QSharedPointer<CFunction> createFuncion(Return(Class::*f)(Args...) const noexcept, Extra... extra)
{
    auto func = [f](Class *inst, Args... args)->Return { return (inst->*f)(args...); };
    return createFuncionImpl<decltype(func), Extra...>(func, extra...);
}
#endif

//single
template<class type>
void createSlotParams(QVariantList &)
{
    return;
}

template <class type,typename T,typename... Args>
void createSlotParams(QVariantList &params,const T &v,Args... args)
{
    params.push_back(toVariant(v));
    createSlotParams<type,Args...>(params,args...);
}

template<typename Class, typename... Args>
class CSingleImpl : public CSingle
{
public:    
    virtual void connect(JZNodeObject *obj,JZNodeObject *recv,QString slot)
    {
        auto func_ptr = [recv,slot](Args... args){
           QVariantList params;           
           createSlotParams<int,Args...>(params,args...);
           recv->onSig(slot,params);
        };

        Class *cobj = (Class*)obj->cobj();
        auto conn = cobj->connect(cobj,single,recv,func_ptr);

        ConnectInfo info;
        info.send = obj;
        info.recv = recv;
        info.slot = slot;
        info.conn = conn;
        m_connects.push_back(info);
    }

    virtual void disconnect(JZNodeObject *obj,JZNodeObject *recv,QString slot)
    {
        int index = getConnectIndex(obj,recv,slot);
        if(index == -1)
            return;

        Class *cobj = (Class*)obj->cobj();
        cobj->disconnect(m_connects[index].conn);
        m_connects.removeAt(index);
    }

    void (Class::*single)(Args...); 

protected:
    struct ConnectInfo
    {
        JZNodeObject *send;
        JZNodeObject *recv;
        QString slot;
        QMetaObject::Connection conn;
    };

    int getConnectIndex(JZNodeObject *obj,JZNodeObject *recv,QString slot)
    {
        for(int i = 0; i < m_connects.size(); i++)
        {
            auto &info = m_connects[i];
            if(obj == info.send && recv == info.recv && slot == info.slot)
                return i;
        }
        return -1;
    }

    QList<ConnectInfo> m_connects;
};

template<typename Class,typename PrivateSingle,typename... Args>
class CPrivateSingleImpl : public CSingle
{
public:    
    virtual void connect(JZNodeObject *obj,JZNodeObject *recv,QString slot)
    {
        auto func = [recv,slot](Args... args){
           QVariantList params;           
           createSlotParams<int,Args...>(params,args...);
           recv->onSig(slot,params);
        };

        Class *cobj = (Class*)obj->cobj();
        cobj->connect(cobj,single,recv,func);  
    }

    virtual void disconnect(JZNodeObject *obj,JZNodeObject *recv,QString slot)
    {
        Class *cobj = (Class*)obj->cobj();
        cobj->disconnect(cobj, single, recv, nullptr);
    }

    void (Class::*single)(PrivateSingle, Args...);
};

template<typename T>
int registEnum(QString name,int id = -1)
{    
    QStringList keys;
    QVector<int> values;
    
    auto meta = QMetaEnum::fromType<T>();
    int count = meta.keyCount();
    for (int i = 0; i < count; i++)
    {
        keys << meta.key(i);
        values << meta.value(i);
    }

    JZNodeEnumDefine define;
    define.init(meta.name(),keys,values);
    if(id != -1)
        define.setType(id);
    return JZNodeObjectManager::instance()->registCEnum(define,typeid(T).name());
}

template<class Class>
class WidgetEventHelper : public Class
{
public:
    using T = WidgetEventHelper<Class>;

    template<typename Return,typename FuncClass, typename... Args>
	static bool isNewFunction(Return (FuncClass::*func)(Args...))
	{
        Q_UNUSED(func);
		return std::is_same<Class,FuncClass>::value;
	}

    static bool isNewPaintEvent() { return isNewFunction(&WidgetEventHelper::paintEvent); }
    static decltype(WidgetEventHelper::paintEvent) getPaintEvent() { return &WidgetEventHelper::paintEvent; }

    static bool isNewShowEvent() { return isNewFunction(&WidgetEventHelper::showEvent); }
    static decltype(WidgetEventHelper::showEvent) getShowEvent() { return &WidgetEventHelper::showEvent; }
    
    static bool isNewResizeEvent() { return isNewFunction(&WidgetEventHelper::resizeEvent); }
    static decltype(WidgetEventHelper::resizeEvent) getResizeEvent() { return &WidgetEventHelper::resizeEvent; }
    
    static bool isNewCloseEvent() { return isNewFunction(&WidgetEventHelper::closeEvent); }
    static decltype(WidgetEventHelper::closeEvent) getCloseEvent() { return &WidgetEventHelper::closeEvent; }
    
    static bool isNewKeyPressEvent() { return isNewFunction(&WidgetEventHelper::keyPressEvent); }
    static decltype(WidgetEventHelper::keyPressEvent) getKeyPressEvent() { return &WidgetEventHelper::keyPressEvent; }
    
    static bool isNewKeyReleaseEvent() { return isNewFunction(&WidgetEventHelper::keyReleaseEvent); }
    static decltype(WidgetEventHelper::keyReleaseEvent) getKeyReleaseEvent() { return &WidgetEventHelper::keyReleaseEvent; }
    
    static bool isNewMousePressEvent() { return isNewFunction(&WidgetEventHelper::mousePressEvent); }
    static decltype(WidgetEventHelper::mousePressEvent) getMousePressEvent() { return &WidgetEventHelper::mousePressEvent; }
    
    static bool isNewMouseMoveEvent() { return isNewFunction(&WidgetEventHelper::mouseMoveEvent); }
    static decltype(WidgetEventHelper::mouseMoveEvent) getMouseMoveEvent() { return &WidgetEventHelper::mouseMoveEvent; }

    static bool isNewMouseReleaseEvent() { return isNewFunction(&WidgetEventHelper::mouseReleaseEvent); }
    static decltype(WidgetEventHelper::mouseReleaseEvent) getMouseReleaseEvent() { return &WidgetEventHelper::mouseReleaseEvent; }
};

template<class ret_type>
ret_type getReturn(const QVariantList &list)
{
    return fromVariant<ret_type>(list[0]);
}

template<>
void getReturn(const QVariantList &);

#define JZBIND_OVERRIDE_IMPL(ret_type, func, ...)              \
    do                                                         \
    {                                                          \
        auto jzobj = toJZObject(this->property("JZObject"));   \
        Q_ASSERT(!jzobj || !jzobj->function(#func));           \
                                                               \
        auto func_def = jzobj->function(#func);                \
        QVariantList input,output;                             \
        input.push_back(QVariant::fromValue(jzobj));           \
        toVariantList<int>(input,__VA_ARGS__);                 \
        JZScriptInvoke(func_def->fullName(),input,output);     \
        return getReturn<ret_type>(output);                    \
    } while (0)
    
template<class Class>
class WidgetWrapper : public Class
{  
public:
    void paintEvent(QPaintEvent *event) override
    {
        JZBIND_OVERRIDE_IMPL(void,paintEvent,event);
    }
    void showEvent(QShowEvent *event) override
    {
        JZBIND_OVERRIDE_IMPL(void,showEvent,event);
    }
    void resizeEvent(QResizeEvent *event)override
    {
        JZBIND_OVERRIDE_IMPL(void,resizeEvent,event);
    }
    void closeEvent(QCloseEvent *event) override
    {
        JZBIND_OVERRIDE_IMPL(void,closeEvent,event);
    }
    void keyPressEvent(QKeyEvent *event) override
    {
        JZBIND_OVERRIDE_IMPL(void,keyPressEvent,event);
    }
    void keyReleaseEvent(QKeyEvent *event) override
    {
        JZBIND_OVERRIDE_IMPL(void,keyReleaseEvent,event);
    }
    void mousePressEvent(QMouseEvent *event) override
    {
        JZBIND_OVERRIDE_IMPL(void,mousePressEvent,event);
    }
    void mouseMoveEvent(QMouseEvent *event) override
    {
        JZBIND_OVERRIDE_IMPL(void,mouseMoveEvent,event);
    }
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        JZBIND_OVERRIDE_IMPL(void,mouseReleaseEvent,event);
    }
};

template<class Class>
class ClassBind
{
public:
    ClassBind(int typeId,QString name, QString super = QString())        
    {
        m_super = super;
        m_define.className = name;
        m_define.superName = super;
        m_define.isCObject = true;
        m_define.id = JZNodeObjectManager::instance()->delcareCClass(m_define.className, typeid(Class).name(), typeId);

        initCreate(std::is_abstract<Class>());
        initCopy(std::is_copy_constructible<Class>());
        m_define.cMeta.isAbstract = std::is_abstract<Class>();        
    }

    ClassBind(QString name, QString super = QString())
        :ClassBind(-1, name, super)
    {        
    }    

    template<typename Return, typename... Args,typename... Extra>
    JZFunctionDefine *def(QString name,bool isflow,Return (Class::*f)(Args...),Extra ...extra)
    {
        auto impl = createFuncion(f,extra...);
        return registFunction(name,isflow,impl);
    }

    template<typename Return, typename... Args,typename... Extra>
    JZFunctionDefine *def(QString name,bool isflow,Return (Class::*f)(Args...) const,Extra ...extra)
    {
        auto impl = createFuncion(f,extra...);
        return registFunction(name,isflow,impl);
    }

    template <typename Func,typename... Extra>
    JZFunctionDefine *def(QString name,bool isflow,Func f,Extra ...extra)
    {
        auto impl = createFuncion(f,extra...);
        return registFunction(name,isflow,impl);
    }       

    template<typename... Args>
    void defSingle(QString name, void (Class::*f)(Args...))
    {
        registFunction(name,true,createFuncion(f));

        JZSingleDefine single;
        single.name = name;
        single.className = m_define.className;

        auto *impl = new CSingleImpl<Class,Args...>();        
        impl->single = f;
        single.csingle = impl;
        
        JZParamDefine sender;
        sender.name = "sender";
        single.paramOut.push_back(sender);

        QStringList args;
        getFunctionParam<int, Args...>(args);
        for (int i = 0; i < args.size(); i++)
        {
            int dataType = JZNodeType::typeidToType(args[i]);

            JZParamDefine def;
            def.name = "output" + QString::number(i);
            def.type = JZNodeType::typeToName(dataType);
            single.paramOut.push_back(def);
        }
        m_define.singles.push_back(single);
    }

    template<typename... Args>
    void defPrivateSingle(QString name, void (Class::*f)(Args...))
    {                
        JZSingleDefine single;
        single.name = name;
        single.className = m_define.className;

        auto *impl = new CPrivateSingleImpl<Class, Args...>();
        impl->single = f;
        single.csingle = impl;
        
        JZParamDefine sender;
        sender.name = "sender";         
        single.paramOut.push_back(sender);

        QStringList args;
        getFunctionParam<int, Args...>(args);
        for (int i = 1; i < args.size(); i++)
        {
            int dataType = JZNodeType::typeidToType(args[i]);

            JZParamDefine def;
            def.name = "output" + QString::number(i);            
            def.type = JZNodeType::typeToName(dataType);
            single.paramOut.push_back(def);
        }
        m_define.singles.push_back(single);
    }

    void regist()
    {
        auto func_inst = JZNodeFunctionManager::instance();
        //replace with real id
        for (int i = 0; i < m_define.singles.size(); i++)
        {
            auto &single = m_define.singles[i];
            if (single.paramOut[0].dataType() == Type_none)
                single.paramOut[0].type = JZNodeType::typeToName(m_define.id);
        }
        for (int func_idx = 0; func_idx < m_funcList.size(); func_idx++)
        {
            auto f = m_funcList[func_idx].data();
            func_inst->registCFunction(*f, m_funcImplList[func_idx]);
            m_define.functions.push_back(*f);
        }
        //regist
        JZNodeObjectManager::instance()->replace(m_define);
        setQObjectType(std::is_base_of<QObject, Class>());
    }

protected:
    template<class Function>
    void defEvent(QString name, Function f)
    {
        auto func = def(name, true, f);
        func->isVirtualFunction = true;
        func->isProtected = true;
    }

    void defWidgetEvent(std::true_type)
    {
        using W = WidgetEventHelper<Class>;

        m_define.cMeta.create = &createClass<WidgetWrapper<Class>>;
        m_define.cMeta.destory = &destoryClass<WidgetWrapper<Class>>;

        if(W::isNewPaintEvent()) defEvent("paintEvent", W::getPaintEvent());
        if(W::isNewShowEvent()) defEvent("showEvent", W::getShowEvent());
        if(W::isNewResizeEvent()) defEvent("resizeEvent", W::getResizeEvent());
        if(W::isNewCloseEvent()) defEvent("closeEvent", W::getCloseEvent());
        if(W::isNewKeyPressEvent()) defEvent("keyPressEvent", W::getKeyPressEvent());
        if(W::isNewKeyReleaseEvent()) defEvent("keyReleaseEvent", W::getKeyReleaseEvent());
        if(W::isNewMousePressEvent()) defEvent("mousePressEvent", W::getMousePressEvent());
        if(W::isNewMouseMoveEvent()) defEvent("mouseMoveEvent", W::getMouseMoveEvent());
        if(W::isNewMouseReleaseEvent()) defEvent("mouseReleaseEvent", W::getMouseReleaseEvent());
    }

    void defWidgetEvent(std::false_type)
    {

    }

    void initCreate(std::true_type)
    {
        m_define.cMeta.create = createClassAssert;
        m_define.cMeta.destory = destoryClassAssert;
    }

    void initCreate(std::false_type)
    {
        initCreateDefault(std::is_default_constructible<Class>());
    }

    void initCreateDefault(std::true_type)
    {
        m_define.cMeta.create = &createClass<Class>;
        m_define.cMeta.destory = &destoryClass<Class>;
        defWidgetEvent(std::is_base_of<QWidget, Class>());
    }

    void initCreateDefault(std::false_type)
    {
        m_define.cMeta.create = createClassAssert;
        m_define.cMeta.destory = &destoryClass<Class>;
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

    JZFunctionDefine *registFunction(QString name,bool isflow,QSharedPointer<CFunction> cfunc)
    {
        JZFunctionDefine *f = new JZFunctionDefine();
        f->name = name;
        f->className = m_define.className;
        f->isFlowFunction = isflow;        
        f->isCFunction = true;
        f->updateParam(cfunc.data());
        if (cfunc->args.size() > 0 && JZNodeType::typeidToType(cfunc->args[0]) == m_define.id)
            f->paramIn[0].name = "this";        

        m_funcList.push_back(QSharedPointer<JZFunctionDefine>(f));
        m_funcImplList.push_back(cfunc);
        return f;
    }

    void setQObjectType(std::true_type)
    {
        QString className = Class::staticMetaObject.className();
        JZNodeObjectManager::instance()->setQObjectType(className,m_define.id);
    }

    void setQObjectType(std::false_type)
    {

    }

    QString m_super;
    JZNodeObjectDefine m_define;    
    QList<QSharedPointer<JZFunctionDefine>> m_funcList;
    QList<QSharedPointer<CFunction>> m_funcImplList;
};


}

#endif
