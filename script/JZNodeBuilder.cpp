#include <QScopeGuard>
#include "JZNodeBuilder.h"
#include "JZParamItem.h"
#include "JZUiItem.h"
#include "JZClassItem.h"
#include "LogManager.h"
#include "JZNodeUtils.h"
#include "JZContainer.h"

//JZNodeCustomBuild
class JZNodeGlobalBuild : public JZNode
{
public:
    JZNodeGlobalBuild()
    {
        m_type = Node_custom;
        addFlowIn();
        addFlowOut();
    }

    bool compiler(JZNodeCompiler *c, QString &error)
    {
        c->addNodeEnter(m_id);

        auto project = m_file->project();
        auto global_params = project->globalVariableList();
        for (int i = 0; i < global_params.size(); i++)
        {
            auto def = project->globalVariable(global_params[i]);
            int data_type = project->environment()->nameToType(def->type);
            c->addAlloc(JZNodeIRAlloc::Heap, def->name, data_type);
            c->addInitVariable(irRef(def->name), data_type, def->value);
        }
        c->addFlowOutput(m_id);
        return true;
    }
};

//JZNodeConstructBuild
class JZNodeConstructBuild : public JZNode
{
public:
    JZNodeConstructBuild()
    {
        m_type = Node_custom;
        addFlowIn();
        addFlowOut();
    }

    bool compiler(JZNodeCompiler* c, QString& error)
    {
        c->addNodeEnter(m_id);

        QString class_name = m_file->getClassItem()->className();

        auto &class_info = build->m_classConstructor[class_name];
        for (int i = 0; i < class_info.infoList.size(); i++)
        {
            auto& info = class_info.infoList[i];
            QString function = info.function;
            QList<JZNodeIRParam> in;
            for (int param_idx = 0; param_idx < info.irList.size(); param_idx++)
            {
                in << info.irList[param_idx];
            }
            QList<JZNodeIRParam> out;
            c->addCall(function, in, out);
        }

        return true;
    }

    JZNodeBuilder* build;
};


//JZNodeBuilder
JZNodeBuilder::JZNodeBuilder()
{
    m_logEnable = true;
    m_project = nullptr;
    m_build = false;
    m_stopBuild = false;
}

JZNodeBuilder::~JZNodeBuilder()
{

}

void JZNodeBuilder::setMute(bool mute)
{
    m_logEnable = !mute;
}

void JZNodeBuilder::setProject(JZProject *project)
{
    clear();
    m_project = project;
    m_compiler.setBuilder(this);
}

JZProject *JZNodeBuilder::project()
{
    return m_project;
}

bool JZNodeBuilder::isError() const
{
    return m_error;
}

QString JZNodeBuilder::error() const
{
    QString result = m_checkError;
    if (!result.isEmpty())
        result += "\n";

    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        auto err_it = it->compilerInfo.nodeError.begin();
        while(err_it != it->compilerInfo.nodeError.end())
        {
            result += "node" + QString::number(err_it.key()) + ":" + err_it.value() + "\n";
            err_it++;
        }

        it++;
    }
    return result;
}

void JZNodeBuilder::clear()
{
    m_checkError.clear();
    m_build = false;
    m_stopBuild = false;
    m_error = false;
    m_scripts.clear();
    m_classConstructor.clear();
}

void JZNodeBuilder::log(int level, const QString &text)
{
    if (!m_logEnable)
        return;

    JZLogManager::instance()->log(Log_Compiler, level, text);
}

void JZNodeBuilder::logE(const QString& text)
{
    log(LOG_ERROR, text);
}

QMap<QString, CompilerResult> JZNodeBuilder::compilerResult()
{
    QMap<QString, CompilerResult> ret;

    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        ret[it.key()] = it->compilerInfo;
        it++;
    }
    return ret;
}

const CompilerResult *JZNodeBuilder::compilerInfo(JZScriptItem *file) const
{
    auto it = m_scripts.find(file->itemPath());
    if (it == m_scripts.end())
        return nullptr;

    return &it->compilerInfo;
}

void JZNodeBuilder::setScriptInclude(QList<JZScriptItem*> extList)
{
    m_scriptInclude = extList;
}

void JZNodeBuilder::setScriptExclude(QList<JZScriptItem*> extList)
{
    m_scriptExclude = extList;
}

bool JZNodeBuilder::buildScript(JZScriptItem *scriptFile,JZNodeScript* script)
{
    if(m_stopBuild)
    {
        return false;
    }

    QString path = scriptFile->itemPath();
    if(!scriptFile->itemPath().startsWith("/tmp"))
        log(LOG_INFO, "build " + scriptFile->itemPath());

    bool ret = m_compiler.build(scriptFile, script);
    m_scripts[path].compilerInfo = m_compiler.compilerResult();
    if(!ret)
    {        
        m_error = true;        
        return false;
    }
    
    return true;
}

bool JZNodeBuilder::build(JZNodeProgram *program)
{        
    {
        QMutexLocker locker(&m_mutex);
        m_build = true;
        m_stopBuild = false;
    }

    m_error = false;
    auto cleanup = qScopeGuard([this]{ 
        QMutexLocker locker(&m_mutex);
        m_build = false;
        m_stopBuild = false;

        if(m_error)
            log(LOG_INFO,"build failed");
        else
            log(LOG_INFO,"build finish");
    });

    auto makeParamLink = [](QString tips, QString path, bool ui, int row)->QString
    {
        QVariantMap  args;
        if (ui)
            args["type"] = "ui";
        else
            args["type"] = "param";

        args["row"] = row;
        return JZNodeUtils::makeLink(tips, path, args);
    };

    clear();    
    
    m_project->registType();
    m_program = program;        
    m_program->clear();    

    log(LOG_INFO, "start build");
    
    auto env = m_project->environment();
    auto obj_inst = env->objectManager();
    
    JZNodeTypeMeta type_meta;    
    auto list = m_project->globalVariableList();
    for(int i = 0; i < list.size(); i++)
    {
        QString name = list[i];
        auto def = m_project->globalVariable(name);
        m_program->m_variables[name] = *def;

        QString error;
        if (!m_compiler.checkParamDefine(def, error))
        {
            auto global_item = m_project->globalDefine();
            logE(makeParamLink(error, global_item->itemPath(),false, i));
            m_checkError += error + "\n";
            m_error = true;
        }
    }
    
    auto class_list = m_project->itemList("./",ProjectItem_class);
    for(int cls_idx = 0; cls_idx < class_list.size(); cls_idx++)
    {
        JZScriptClassItem *class_item = dynamic_cast<JZScriptClassItem*>(class_list[cls_idx]);
        auto obj_def = obj_inst->meta(class_item->className());
        type_meta.objectList << *obj_def;
                
        QString error;
        if (!obj_def->check(error))
        {
            logE(JZNodeUtils::makeLink(error, class_item->itemPath()));
            m_checkError += error + "\n";
            m_error = true;
        }

        JZParamItem *param = class_item->paramFile();
        auto var_list = param->variableList();
        for(int i = 0; i < var_list.size(); i++)
        {
            auto var_def = param->variable(var_list[i]);            
            if (!m_compiler.checkParamDefine(var_def, error))
            {
                logE(makeParamLink(error, param->itemPath(), false, i));
                m_checkError += error + "\n";
                m_error = true;
            }
        }

        auto bind_list = param->bindVariableList();
        for (int i = 0; i < bind_list.size(); i++)
        {
            auto bind = param->bindVariable(bind_list[i]);
            if (!obj_def->param(bind->path))
            {
                error = JZNodeCompiler::errorString(Error_noClassMember, { obj_def->className,bind->path});
                logE(makeParamLink(error, param->itemPath(),true, i));
                m_checkError += error + "\n";
                m_error = true;
            }
        }
    }
    if(m_error)
        return false;
        
    auto function_list = m_project->itemList("./", ProjectItem_scriptItem);
    for(auto ext : m_scriptInclude)
        function_list << ext;
    for (int i = 0; i < function_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(function_list[i]);
        if (m_scriptExclude.contains(script))
            continue;
        if (script->nodeCount() == 0)
            continue;

        JZScriptClassItem* class_item = script->getClassItem();
        JZNodeScriptPtr script_impl = JZNodeScriptPtr(new JZNodeScript());
        if (!buildScript(script, script_impl.data()))
            return false;

        m_scripts[script->itemPath()].script = script_impl;

        auto func_def = script->function();
        if (class_item && !class_item->memberFunction(func_def.name))
        {
            auto cls_def = type_meta.object(class_item->className());
            cls_def->addFunction(func_def);
        }
        if(func_def.className.isEmpty())
            type_meta.functionList << func_def;        
    }    

    m_program->m_typeMeta = type_meta;
    if(!link())
        return false;

    return true;
}

bool JZNodeBuilder::buildCustom(JZFunctionDefine func, JZNode* custom_node, JZNodeScript * script_impl)
{
    JZScriptItem *file = new JZScriptItem(JZScriptItem::Function);
    file->setFunction(func);

    JZProjectTempGuard guard(m_project, file, JZProjectTempGuard::RemoveItem);
    if (!func.className.isEmpty())
        guard.setClass(func.className);

    auto start = file->getNode(0);
    file->addNode(custom_node);
    file->addConnect(start->flowOutGemo(), custom_node->flowInGemo());
 
    if(!buildScript(file, script_impl))
        return false;
    
    return true;
}

void JZNodeBuilder::stopBuild()
{   
    {
        QMutexLocker locker(&m_mutex);
        if(!m_build)
            return;
    
        m_stopBuild = true;
    }

    //wait
    while(true)
    {
        {
            QMutexLocker locker(&m_mutex);
            if(!m_build)
                return;
        }
        
        QThread::msleep(10);
    }
}

bool JZNodeBuilder::isBuildInterrupt()
{
    QMutexLocker locker(&m_mutex);
    return m_stopBuild;
}

void JZNodeBuilder::addClassInitFunction(QString class_name, ClassInitInfo info)
{
    if (!m_classConstructor.contains(class_name))
        m_classConstructor[class_name] = ClassConstructor();

    auto& func = m_classConstructor[class_name];
    func.infoList << info;
}

bool JZNodeBuilder::initGlobal()
{
    // init variable
    JZNodeGlobalBuild* global = new JZNodeGlobalBuild();

    JZFunctionDefine global_func;
    global_func.name = "__init__";
    global_func.isFlowFunction = true;
    
    JZNodeScriptPtr script_impl = JZNodeScriptPtr(new JZNodeScript());
    if (!buildCustom(global_func, global, script_impl.data()))
        return false;

    m_program->m_typeMeta.functionList << global_func;
    m_scripts[script_impl->itemPath].script = script_impl;

    return true;
}


bool JZNodeBuilder::initConstructor()
{
    if (m_classConstructor.size() == 0)
        return true;

    JZNodeScriptPtr con_script = JZNodeScriptPtr(new JZNodeScript());
    con_script->itemPath = "__ClassConstructorImpl__";

    auto it = m_classConstructor.begin();
    while (it != m_classConstructor.end())
    {
        auto class_meta = m_program->m_typeMeta.object(it.key());
        JZFunctionDefine function = class_meta->initMemberFunction("__init__");

        JZNodeConstructBuild* node = new JZNodeConstructBuild();
        node->build = this;

        JZNodeScriptPtr script_impl = JZNodeScriptPtr(new JZNodeScript());
        if(!buildCustom(function, node, script_impl.data()))
            return false;

        class_meta->addFunction(function);

        JZFunction jz_func;
        jz_func.define = function;
        jz_func.addr = con_script->statmentList.size();
        jz_func.addrEnd = jz_func.addr + script_impl->statmentList.size();
        jz_func.path = con_script->itemPath;

        con_script->statmentList << script_impl->statmentList;
        con_script->functionList << jz_func;
        con_script->functionDebugList << script_impl->functionDebugList[0];        

        it++;
    }
    m_program->m_scripts[con_script->itemPath] = con_script;
    return true;
}

bool JZNodeBuilder::link()
{    
    if (!initGlobal())
    {
        logE("initGlobal failed");
        return false;
    }

    auto it_s = m_scripts.begin();
    while (it_s != m_scripts.end())
    {
        m_program->m_scripts[it_s.key()] = it_s->script;
        it_s++;
    }
    initConstructor();

    return true;
}

