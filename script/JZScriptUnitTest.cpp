#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTimerEvent>
#include "JZNodeCompiler.h"
#include "JZScriptUnitTest.h"
#include "JZNodeFactory.h"
#include "JZNodeEngine.h"
#include "JZNodeBuilder.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeProgramDumper.h"
#include "JZNodeBind.h"
#include "LogManager.h"

static JZScriptUnitTest* g_hook = nullptr;

//JZUnitTestHook
class JZUnitTestHook : public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {        
        int hook_id = engine->getReg(Reg_CallIn).toInt();
        Q_ASSERT(g_hook->hasHook(hook_id));

        int count = engine->regInCount();
        for (int i = 0; i < count; i++)
        {
            int out_id = engine->getReg(Reg_CallIn + i).toInt();
            engine->setReg(Reg_CallOut, g_hook->hookValue(out_id));
        }
    }
};

//JZUnitWidgetHook
class JZUnitWidgetHook : public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        auto env = engine->environment();
        for (int i = 0; i < define.paramOut.size(); i++)
        {
            QVariant v = env->defaultValue(env->nameToType(define.paramOut[i].type));
            engine->setReg(Reg_CallOut + i, v);
        }
    }

    JZFunctionDefine define;
};


//JZUnitTestFinish
class JZUnitTestFinish : public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine* engine) override
    {
        g_hook->setFinish(true);
    }
};

//JZScriptItemDepend    
JZScriptItemDepend::JZScriptItemDepend()
{
    originScript = nullptr;
    initExtScript = nullptr;
    triggerScript = nullptr;
    unitScript = nullptr;
    isTrigger = false;    

    status = None;
}

void JZScriptItemDepend::init(JZScriptItem *script)
{
    originScript = script;
    initExtScript->clear();
    triggerScript->clear();
    unitScript->clear();

    paramList.clear();
    functionList.clear();
    error = QString();

    function = script->function();
    status = None;
}

bool JZScriptItemDepend::isError()
{
    return !error.isEmpty();
}

void JZScriptItemDepend::setParam(int id, const QVariant &value)
{
    for (int i = 0; i < paramList.size(); i++)
    {
        if (paramList[i].node_id == id)
            paramList[i].value = value;
    }
}

//JZNodeUnitTest
JZNodeUnitTest::JZNodeUnitTest()
{
    m_type = Node_unitTest;
    m_name = "unitTest";
    hook = nullptr;
}

JZNodeUnitTest::~JZNodeUnitTest()
{
}

void JZNodeUnitTest::copyFrom(const JZNode* node)
{
    m_pinList.clear();
    
    m_id = node->id();
    int n = node->pinCount();
    for (int i = 0; i < n; i++)
    {
        auto pin = node->pinByIndex(i);
        m_pinList.push_back(*pin);
    }
}

bool JZNodeUnitTest::compiler(JZNodeCompiler* c, QString& error)
{   
    if (isFlowNode())
    {
        if (!c->addFlowInput(m_id, error))
            return false;
    }
    else
    {
        if (!c->addDataInput(m_id, error))
            return false;
    }

    QList<int> param_out_list = paramOutList();
    QList<JZNodeIRParam> in, out;    
    for (int i = 0; i < param_out_list.size(); i++)
    {
        int out_id = c->paramId(m_id, param_out_list[i]);
        out << irId(out_id);

        in << irLiteral(out_id);

        int out_type = JZNodeType::variantType(hook->hookValue(out_id));
        c->setPinType(m_id, param_out_list[i], out_type);
    }    
    c->addCall("JZUnitTestHook",in, out);
    if (isFlowNode())
        c->addFlowOutput(m_id);

    return true;
}

//JZScriptUnitTestVistor
JZScriptUnitTestVistor::JZScriptUnitTestVistor()
{
    m_depend = nullptr;
}

void JZScriptUnitTestVistor::updateDepend(JZScriptItemDepend *depend)
{
    m_depend = depend;
    setScript(m_depend->originScript);
    visitor();
}

//JZScriptNomarlVistor
JZScriptHookVistor::JZScriptHookVistor()
{
}

void JZScriptHookVistor::visitorSelf(JZNode *node)
{
    if(node->type() == Node_param)
    {
        JZScriptItemDepend::ParamDepend d;
        d.node_id = node->id();
        m_depend->paramList.push_back(d);
    }
    else if(node->type() == Node_function)
    {
        JZScriptItemDepend::FunctionDepend d;
        d.node_id = node->id();
        m_depend->functionList.push_back(d);
    }
}

//JZScriptUnitTest
JZScriptUnitTest::JZScriptUnitTest()
{
    m_script = new JZScriptItem(JZScriptItem::Function);
    m_initExtScript = new JZScriptItem(JZScriptItem::Function);
    m_triggerScript = new JZScriptItem(JZScriptItem::Function);

    m_depend.initExtScript = m_initExtScript;
    m_depend.triggerScript = m_triggerScript;
    m_depend.unitScript = m_script;    

    m_project = nullptr;

    m_engine = new JZNodeEngine(this);
    connect(m_engine, &JZNodeEngine::sigRuntimeError, this, &JZScriptUnitTest::onRuntimeError);
    hookEngine();
}

JZScriptUnitTest::~JZScriptUnitTest()
{
    delete m_initExtScript;
    delete m_triggerScript;
    delete m_script;
}

void JZScriptUnitTest::setProject(JZProject* project)
{    
    m_project = project;
}

void JZScriptUnitTest::setScript(JZScriptItem *script)
{
    m_depend.init(script);

    JZScriptHookVistor visitor;
    visitor.updateDepend(&m_depend);

    JZScriptUnitTestManager::instance()->updateDepend(&m_depend);
}

void JZScriptUnitTest::hookEngine()
{
    auto env = m_engine->environment();
    auto obj_inst = env->objectManager();
    auto func_inst = env->functionManager();
    auto class_list = obj_inst->getClassList();
    for (int cls_idx = 0; cls_idx < class_list.size(); cls_idx++)
    {
        if (!obj_inst->isInherits(class_list[cls_idx], "QWidget"))
            continue;

        JZNodeObjectDefine def = *obj_inst->meta(class_list[cls_idx]);
        def.cMeta.create = jzbind::createClass<QObject>;
        def.cMeta.destory = jzbind::destoryClass<QObject>;

        for (int func_idx = 0; func_idx < def.functions.size(); func_idx++)
        {            
            JZFunction func_impl = *func_inst->functionImpl(def.functions[func_idx].fullName());
            func_impl.cfunc.clear();

            JZUnitWidgetHook *widget_hook = new JZUnitWidgetHook();
            widget_hook->define = def.functions[func_idx];
            func_impl.builtIn = BuiltInFunctionPtr(widget_hook);

            func_inst->replaceFunctionImpl(func_impl);
        }
        obj_inst->replace(def);
    }
}

void JZScriptUnitTest::initRuntime()
{
    auto env = m_project->environment();
    auto meta = m_program.typeMeta();
    for (int i = 0; i < meta.objectList.size(); i++)
    {
        auto &cls_def = meta.objectList[i];
        if (env->isInherits(cls_def.className,"QWidget"))
        {
            cls_def.superName = "QObject";
            cls_def.widgetDefine = JZNodeObjectWidgetDefine();
        }
    }
    m_program.setTypeMeta(meta);
}

bool JZScriptUnitTest::hasHook(int id)
{
    return m_hookValues.contains(id);
}

QVariant JZScriptUnitTest::hookValue(int id)
{
    return m_hookValues.value(id);
}

JZNodeEngine* JZScriptUnitTest::engine()
{
    return m_engine;
}

JZScriptItemDepend *JZScriptUnitTest::depend()
{
    return &m_depend;
}

void JZScriptUnitTest::dump(QString dir)
{
    JZNodeProgramDumper dumper;
    dumper.init(m_project, &m_program);
    dumper.dump(dir);
}

bool JZScriptUnitTest::init()
{
    auto class_item = m_depend.originScript->getClassItem();
    if (!class_item || !class_item->memberFunction("init"))
    {
        m_error = "no init function";
        return false;
    }

    JZProjectTempGuard guard(m_project, { m_script, m_initExtScript, m_triggerScript }, JZProjectTempGuard::TakeItem);
    auto meta = m_project->environment()->meta(class_item->className());
    m_initExtScript->setFunction(meta->initMemberFunction("UnitTestInitExt"));
    m_triggerScript->setFunction(meta->initMemberFunction("UnitTestTrigger"));
    guard.setClass(class_item->className());

    //replace
    QByteArray buffer = m_depend.originScript->toBuffer();
    m_script->fromBuffer(buffer);
    m_script->loadFinish();

    QString func_name = m_script->function().fullName();
    m_hookValues.clear();
    auto replace_node = [this](int id, const QVariant& value)
    {
        const JZNode* node = m_depend.originScript->getNode(id);

        int hook_id = JZNodeCompiler::paramId(id, node->paramOut(0));
        m_hookValues[hook_id] = value;

        JZNodeUnitTest* new_node = new JZNodeUnitTest();
        new_node->hook = this;
        new_node->copyFrom(node);
        m_script->removeNodeOnly(node->id());
        m_script->insertNode(new_node);
    };

    //set hook value
    for (int i = 0; i < m_depend.paramList.size(); i++)
    {
        auto& p = m_depend.paramList[i];
        if (p.value.isValid())
            replace_node(p.node_id, p.value);
    }
    for (int i = 0; i < m_depend.functionList.size(); i++)
    {
        auto& p = m_depend.functionList[i];
        if (p.value.isValid())
            replace_node(p.node_id, p.value);
    }

    JZNodeFunction* set_finish = new JZNodeFunction();
    set_finish->setFunction("JZUnitTestFinish");
    m_script->addNode(set_finish);
    m_script->insertFlow(m_script->startNode(), set_finish);

    //build
    JZNodeBuilder builder;
    builder.setProject(m_project);
    builder.setScriptInclude({ m_script,m_initExtScript, m_triggerScript });
    builder.setScriptExclude({ m_depend.originScript });

    if (!builder.build(&m_program))
    {
        m_error = "build failed";
        return false;
    }
    JZNodeScriptPtr script = m_program.takeScript(m_script->itemPath());
    script->itemPath = m_depend.originScript->itemPath();
    for (int i = 0; i < script->functionList.size(); i++)
        script->functionList[i].path = script->itemPath;
    m_program.addScript(script);
    
    initRuntime();
    
    m_engine->setProgram(&m_program);
    m_engine->init();    

    JZNodeObject* object = m_engine->environment()->objectManager()->create(class_item->classType());
    m_object = JZNodeObjectPointer(object, true);    
    if (!m_depend.function.className.isEmpty())
    {
        QVariantList in, out;
        QString init_function = m_depend.function.className + "::init";
        in << QVariant::fromValue(m_object);
        if (!m_engine->call(init_function, in, out))
        {
            updateStatus(JZScriptItemDepend::Failed);            
            m_engine->deinit();
            return false;
        }
    }

    g_hook = this;
    return true;
}
    
void JZScriptUnitTest::deinit()
{
    if (m_engine->isInit())
    {
        m_engine->deinit();
        m_object = JZNodeObjectPointer();        
        g_hook = nullptr;
    }
}

void JZScriptUnitTest::start()
{            
    g_hook = this;
    QVariantList unit_in = m_depend.input;
    QString trigger_func;
    auto func_def = m_depend.function;
    if (func_def.isMemberFunction())
    {
        unit_in.insert(0, QVariant::fromValue(m_object));
        trigger_func = func_def.className + "::UnitTestTrigger";
    }

    m_depend.status = JZScriptItemDepend::Running;    
    
    QString init_function = m_depend.function.className + "::init";
    bool ret = false;
    if (m_depend.isTrigger)
        ret = m_engine->call(trigger_func, unit_in, m_depend.output);
    else
    {
        if (func_def.fullName() == init_function)
            ret = true;
        else
            ret = m_engine->call(func_def.fullName(), unit_in, m_depend.output);
    }

    if (!ret)
    {
        updateStatus(JZScriptItemDepend::Failed);        
    }
}

void JZScriptUnitTest::stop()
{
    m_engine->stop();
    waitFinish();
    if (m_depend.status != JZScriptItemDepend::Failed)
        m_depend.status = JZScriptItemDepend::Cancel;
    g_hook = nullptr;
}

void JZScriptUnitTest::updateStatus(JZScriptItemDepend::RunStatus status)
{
    m_depend.status = status;
    if (status == JZScriptItemDepend::Failed)
    {
        m_error = m_engine->runtimeError().error;        
    }
}

bool JZScriptUnitTest::isFinish()
{
    if (m_engine->status() == Status_running)
        return false;

    return (m_depend.status != JZScriptItemDepend::Running);
}

void JZScriptUnitTest::setFinish(bool flag)
{
    //这里设置为 success. 如果发生 runtime error, 会覆盖
    m_depend.status = JZScriptItemDepend::Successed;
}

bool JZScriptUnitTest::waitFinish(int timeout)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < timeout)
    {
        if (isFinish())
            return true;

        QThread::msleep(20);
    }
    
    return false;
}

void JZScriptUnitTest::onRuntimeError()
{
    updateStatus(JZScriptItemDepend::Failed);    
}

bool JZScriptUnitTest::run(int timeout)
{
    if (!init())
        return false;

    start();
    QElapsedTimer t;
    t.start();

    while (t.elapsed() < timeout)
    {
        if (isFinish())
            break;

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(20);
    }

    deinit();
    return (m_depend.status == JZScriptItemDepend::Successed);
}


//JZScriptUnitTestManager
JZScriptUnitTestManager *JZScriptUnitTestManager::instance()
{
    static JZScriptUnitTestManager inst;
    return &inst;
}

JZScriptUnitTestManager::JZScriptUnitTestManager()
{
}

JZScriptUnitTestManager::~JZScriptUnitTestManager()
{
    qDeleteAll(m_replaceList);
}

void JZScriptUnitTestManager::initEnv(JZScriptEnvironment* env)
{
    auto func_inst = env->functionManager();

    JZFunctionDefine hook;
    hook.name = "JZUnitTestHook";
    hook.isCFunction = true;
    hook.isFlowFunction = false;
    hook.paramIn.push_back(JZParamDefine("nodeId", "int"));
    hook.paramIn.push_back(JZParamDefine("outId", "args"));
    hook.paramOut.push_back(JZParamDefine("outValue", "arg"));

    auto hook_func = BuiltInFunctionPtr(new JZUnitTestHook());
    func_inst->registBuiltInFunction(hook, hook_func);

    JZFunctionDefine unit_finish_def;
    unit_finish_def.name = "JZUnitTestFinish";
    unit_finish_def.isCFunction = true;
    unit_finish_def.isFlowFunction = true;

    auto unit_finish_func = BuiltInFunctionPtr(new JZUnitTestFinish());
    func_inst->registBuiltInFunction(unit_finish_def, unit_finish_func);

    env->nodeFactory()->registNode(Node_unitTest, createJZNode<JZNodeUnitTest>);
}

void JZScriptUnitTestManager::regist(JZScriptUnitTestVistor *replace)
{
    m_replaceList.push_back(replace);
}

void JZScriptUnitTestManager::updateDepend(JZScriptItemDepend *depend)
{
    for (int i = 0; i < m_replaceList.size(); i++)
    {
        m_replaceList[i]->updateDepend(depend);
        if (depend->isError())
            return;
    }
}