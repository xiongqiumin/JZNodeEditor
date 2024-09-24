#include <QScopeGuard>
#include "JZNodeBuilder.h"
#include "JZParamItem.h"
#include "JZUiFile.h"
#include "JZClassItem.h"
#include "LogManager.h"
#include "JZNodeUtils.h"
#include "JZContainer.h"

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
            int data_type = m_project->environment()->nameToType(def->type);
            c->addAlloc(JZNodeIRAlloc::Heap, def->name, data_type);
            if(!def->value.isEmpty())
                m_compiler.addInitVariable(irRef(def->name), data_type,def->value);
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

    auto makeParamLink = [](QString tips, QString path, bool ui, int row)->QString
    {
        QString args;
        if (ui)
            args = "type=ui";
        else
            args = "type=param";
        args += "&row=" + QString::number(row);
        return JZNodeUtils::makeLink(tips, path, args);
    };

    clear();    
    m_program = program;        
    m_program->clear();    
    
    auto env = m_project->environment();
    JZNodeTypeMeta type_meta;
    auto container_list = m_project->containerList();
    for(int i = 0; i < container_list.size(); i++)
    {
        QString error;
        if(!checkContainer(env,container_list[i],error))
            m_error += error + "\n";
        else
        {
            auto meta = m_project->environment()->objectManager()->meta(container_list[i]);
            Q_ASSERT_X(meta,"Error container:",qUtf8Printable(container_list[i]));

            JZNodeCObjectDelcare cobj;
            cobj.className = meta->className;
            cobj.id = meta->id;
            type_meta.cobjectList << cobj;
        }
    }
    
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
            m_error += makeParamLink(error, global_item->path(),false, i);
        }
    }
    
    auto class_list = m_project->itemList("./",ProjectItem_class);
    for(int cls_idx = 0; cls_idx < class_list.size(); cls_idx++)
    {
        JZScriptClassItem *class_item = dynamic_cast<JZScriptClassItem*>(class_list[cls_idx]);
        auto obj_def = class_item->objectDefine();        
        type_meta.objectList << obj_def;
                
        QString error;
        if(!obj_def.check(error))        
            m_error += error;        

        JZParamItem *param = class_item->paramFile();
        auto var_list = param->variableList();
        for(int i = 0; i < var_list.size(); i++)
        {
            auto var_def = param->variable(var_list[i]);            
            if(!m_compiler.checkParamDefine(var_def,error))
                m_error += makeParamLink(error,param->itemPath(),false, i);
        }

        auto bind_list = param->bindVariableList();
        for (int i = 0; i < bind_list.size(); i++)
        {
            auto bind = param->bindVariable(bind_list[i]);
            if (!var_list.contains(bind->variable))
            {
                error = JZNodeCompiler::errorString(Error_classNoMember, { obj_def.className,bind->variable});
                m_error += makeParamLink(error, param->itemPath(),true, i);
            }
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
    JZScriptItem *file = new JZScriptItem(ProjectItem_scriptFunction);
    file->setName(func.name);
    file->setFunction(func);
    m_project->addTmp(file);
    
    auto cleanup = qScopeGuard([file,this] {
        m_project->removeTmp(file);
    });

    auto start = file->getNode(0);
    JZNodeCustomBuild *custom = new JZNodeCustomBuild();
    custom->buildFunction = buildFunction;
    file->addNode(custom);
    file->addConnect(start->flowOutGemo(), custom->flowInGemo());
 
    if(!buildScript(file))
        return false;
    
    m_program->m_typeMeta.functionList << file->function();
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

