#include <QScopeGuard>
#include "JZNodeBuilder.h"
#include "JZParamItem.h"
#include "JZUiFile.h"
#include "JZClassItem.h"
#include "LogManager.h"
#include "JZNodeUtils.h"
#include "JZContainer.h"

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
    auto func = [this](JZNodeCompiler *c, QString &ret_error)->bool{
        auto global_params = m_project->globalVariableList();
        for(int i = 0; i < global_params.size(); i++)
        {
            auto def = m_project->globalVariable(global_params[i]);
            c->addAlloc(JZNodeIRAlloc::Heap, def->name, def->dataType());
            if(!def->value.isEmpty())
            {
                m_compiler.addInitVariable(irRef(def->name),def->dataType(),def->value);
            }
        }
        return ret_error.isEmpty();
    };

    JZFunctionDefine global_func;
    global_func.name = "__init__";
    global_func.isFlowFunction = true;
    if(!buildCustom(global_func, func))
        return false;

    return true;
}

const CompilerResult *JZNodeBuilder::compilerInfo(JZScriptItem *file) const
{
    auto it = m_scripts.find(file->itemPath());
    if (it == m_scripts.end())
        return nullptr;

    return &it->compilerInfo;
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
    m_scripts[path].compilerInfo = m_compiler.compilerResult();
    if(!ret)
    {
        m_error += m_compiler.error();
        return false;
    }

    return true;
}

bool JZNodeBuilder::build(JZNodeProgram *program)
{    
    Q_ASSERT(JZProject::active() == m_project);
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
    
    JZNodeTypeMeta type_meta;
    auto container_list = m_project->containerList();
    for(int i = 0; i < container_list.size(); i++)
    {
        QString error;
        if(!checkContainer(container_list[i],error))
            m_error += error + "\n";
        else
        {
            auto meta = JZNodeObjectManager::instance()->meta(container_list[i]);
            Q_ASSERT_X(meta,"Error container:",qUtf8Printable(container_list[i]));

            JZNodeCObjectDelcare cobj;
            cobj.className = meta->className;
            cobj.id = meta->id;
            type_meta.cobjectList << cobj;
        }
    }

    auto global_item = m_project->globalDefine();
    auto list = m_project->globalVariableList();
    for(int i = 0; i < list.size(); i++)
    {
        QString name = list[i];
        auto def = m_project->globalVariable(name);
        m_program->m_variables[name] = *def;

        QString error;
        if(!m_compiler.checkVariable(def,error))
            m_error += makeLink(error,global_item->path(), i);
    }
    
    auto class_list = m_project->itemList("./",ProjectItem_class);
    for(int cls_idx = 0; cls_idx < class_list.size(); cls_idx++)
    {
        JZScriptClassItem *class_item = dynamic_cast<JZScriptClassItem*>(class_list[cls_idx]);
        auto obj_def = class_item->objectDefine();        
        type_meta.objectList << obj_def;
                
        JZParamItem *param = class_item->paramFile();
        auto var_list = param->variableList();
        for(int i = 0; i < var_list.size(); i++)
        {
            auto var_def = param->variable(var_list[i]);
            QString error;
            if(!m_compiler.checkVariable(var_def,error))
                m_error += makeLink(error,param->itemPath(),i);
        }
    }
    if(!m_error.isEmpty())
        return false;

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
    type_meta.moduleList = m_project->moduleList();

    m_program->m_typeMeta = type_meta;
    if(!link())
        return false;

    return true;
}

bool JZNodeBuilder::buildCustom(JZFunctionDefine func, std::function<bool(JZNodeCompiler*, QString&)> buildFunction)
{
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
        return false;

    auto it_s = m_scripts.begin();
    while (it_s != m_scripts.end())
    {
        m_program->m_scripts[it_s.key()] = it_s->script;
        it_s++;
    }

    return true;
}

