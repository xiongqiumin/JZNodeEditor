#include <QScopeGuard>
#include "JZNodeBuilder.h"
#include "JZParamItem.h"
#include "JZUiFile.h"
#include "JZClassItem.h"
#include "LogManager.h"

enum {
    build_global,
    build_emptyGlobal,
    build_unitTest,
};

//JZNodeCustomBuild
JZNodeCustomBuild::JZNodeCustomBuild()
{
    m_type = Node_custom;
    addFlowIn();
    addFlowOut();
}

bool JZNodeCustomBuild::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeDebug(m_id);
    if (!buildFunction(c, error))
        return false;

    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeBuilder
JZNodeBuilder::JZNodeBuilder()
{
    m_project = nullptr;
    m_build = false;
    m_stopBuild = false;
}

JZNodeBuilder::~JZNodeBuilder()
{

}

void JZNodeBuilder::setProject(JZProject *project)
{
    m_project = project;
    clear();
}

JZProject *JZNodeBuilder::project()
{
    return m_project;
}

QString JZNodeBuilder::error() const
{
    return m_error;
}

void JZNodeBuilder::clear()
{
    m_build = false;
    m_stopBuild = false;
    
    m_error.clear();
    m_scripts.clear();
}

bool JZNodeBuilder::initGlobal()
{            
    // init variable
    auto func = [this](JZNodeCompiler *c, QString&)->bool{
        auto global_params = m_project->globalVariableList();
        for(int i = 0; i < global_params.size(); i++)
        {
            auto def = m_project->globalVariable(global_params[i]);
            c->addAlloc(JZNodeIRAlloc::Heap, def->name, def->dataType());
            if(!def->value.isEmpty())
            {
                if(!JZNodeType::isObject(def->dataType()))
                    c->addSetVariable(irRef(def->name),irLiteral(JZNodeType::initValue(def->dataType(),def->value)));
                else
                {
                    QString function = def->type + ".__fromString__";
                    QList<JZNodeIRParam> in,out;
                    in << irLiteral(def->value);
                    out << irRef(def->name);
                    c->addCall(function,in,out);
                }
            }
        }
        return true;
    };

    JZFunctionDefine global_func;
    global_func.name = "__init__";
    global_func.isFlowFunction = true;
    if(!buildCustom(global_func, func))
        return false;

    return true;
}

CompilerInfo JZNodeBuilder::compilerInfo(JZScriptItem *file) const
{
    auto it = m_scripts.find(file->itemPath());
    if (it == m_scripts.end())
        return CompilerInfo();

    return it->compilerInfo;
}

bool JZNodeBuilder::buildScript(JZScriptItem *scriptFile)
{
    if(m_stopBuild)
    {
        m_error += "Cancled";
        return false;
    }

    QString path = scriptFile->itemPath();
    LOGI(Log_Compiler, "build " + scriptFile->itemPath());

    m_scripts[path].script = JZNodeScriptPtr(new JZNodeScript());
    JZNodeScript *script = m_scripts[path].script.data();

    bool ret = m_compiler.build(scriptFile, script);
    m_scripts[path].compilerInfo = m_compiler.compilerInfo();
    if(!ret)
    {
        m_error += m_compiler.error();
        LOGE(Log_Compiler, m_compiler.error());
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

    auto cleanup = qScopeGuard([this]{ 
        QMutexLocker locker(&m_mutex);
        m_build = false;
        m_stopBuild = false;
    });

    clear();    
    m_program = program;        
    m_program->clear();    
    
    auto list = m_project->globalVariableList();
    for(int i = 0; i < list.size(); i++)
    {
        QString name = list[i];
        m_program->m_variables[name] = *m_project->globalVariable(name);
    }
    
    JZNodeTypeMeta type_meta;
    auto class_list = m_project->itemList("./",ProjectItem_class);
    for(int i = 0; i < class_list.size(); i++)
    {
        JZScriptClassItem *script = dynamic_cast<JZScriptClassItem*>(class_list[i]);        
        type_meta.objectList << script->objectDefine();
                
        auto params = script->itemList(ProjectItem_param);
        Q_ASSERT(params.size() == 0 || params.size() == 1);
        if(params.size() == 1)
        {
            auto param = dynamic_cast<JZParamItem*>(params[0]);
            m_program->m_binds[script->className()] = param->bindVariables();
        }
    }

    auto container_list = m_project->containerList();
    for(int i = 0; i < container_list.size(); i++)
    {
        auto meta = JZNodeObjectManager::instance()->meta(container_list[i]);
        JZNodeCObjectDelcare cobj;
        cobj.className = meta->className;
        cobj.id = meta->id;
        type_meta.cobjectList << cobj;
    }

    auto bind_list = m_project->itemList("./", ProjectItem_scriptParamBinding);
    for(int i = 0; i < bind_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(bind_list[i]);
        if(!buildScript(script))
            return false;
    }

    auto function_list = m_project->itemList("./", ProjectItem_scriptFunction);
    for (int i = 0; i < function_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(function_list[i]);
        if (!buildScript(script))
            return false;

        auto func_def = script->function();
        if(!func_def.isMemberFunction())
            type_meta.functionList << func_def;        
    }

    m_program->m_typeMeta = type_meta;
    if(!link())
        return false;

    return true;
}

bool JZNodeBuilder::buildCustom(JZFunctionDefine func, std::function<bool(JZNodeCompiler*, QString&)> buildFunction)
{
    JZNodeScriptPtr boot = JZNodeScriptPtr(new JZNodeScript());    

    JZScriptItem file(ProjectItem_scriptFunction);
    file.setName(func.name);
    file.setFunction(func);
    file.setProject(m_project);
    
    auto start = file.getNode(0);
    JZNodeCustomBuild *custom = new JZNodeCustomBuild();
    custom->buildFunction = buildFunction;
    file.addNode(custom);
    file.addConnect(start->flowOutGemo(), custom->flowInGemo());
 
    if(!buildScript(&file))
        return false;
    
    m_program->m_typeMeta.functionList << file.function();
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

bool JZNodeBuilder::link()
{    
    if(!initGlobal())
    {   
        m_error = "initGlobal failed";
        return false;
    }

    auto it_s = m_scripts.begin();
    while (it_s != m_scripts.end())
    {
        m_program->m_scripts[it_s.key()] = it_s->script;
        it_s++;
    }

    return true;
}

