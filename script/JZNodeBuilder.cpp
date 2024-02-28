#include "JZNodeBuilder.h"
#include "JZParamItem.h"
#include "JZUiFile.h"
#include "JZClassItem.h"
#include "LogManager.h"

JZNodeBuilder::JZNodeBuilder()
{

}

JZNodeBuilder::~JZNodeBuilder()
{

}

QString JZNodeBuilder::error() const
{
    return m_error;
}

void JZNodeBuilder::clear()
{
    m_connects.clear();
    m_error.clear();
    m_scripts.clear();
}

void JZNodeBuilder::initGlobal()
{        
    int pc = 0;
    JZNodeScriptPtr boot = JZNodeScriptPtr(new JZNodeScript());
    boot->file = "__init__";    

    m_program->m_scripts[boot->file] = boot;           

    // init variable
    auto global_params = m_program->globalVariables();
    auto it = global_params.begin();
    while(it != global_params.end() )
    {        
        JZNodeIRAlloc *alloc = new JZNodeIRAlloc();      
        alloc->pc = pc++;
        alloc->allocType = JZNodeIRAlloc::Heap;
        alloc->name = it.key();        
        alloc->dataType = it.value().dataType();
        if (it.value().dataType() < Type_object)
            alloc->value = irLiteral(JZNodeType::initValue(it.value().dataType(),it.value().value));
        else
            alloc->value = irLiteral(QVariant::fromValue(JZObjectNull(it.value().dataType())));        

        boot->statmentList.push_back(JZNodeIRPtr(alloc));
        it++;
    }            
    
    JZNodeIR *ir_return = new JZNodeIR(OP_return);
    ir_return->pc = pc++;
    boot->statmentList.push_back(JZNodeIRPtr(ir_return));        

    JZFunction func_def;
    func_def.name = "__init__";
    func_def.file = "__init__";
    func_def.addr = 0;

    boot->functionList.push_back(func_def);
}

bool JZNodeBuilder::build(JZProject *project,JZNodeProgram *program)
{    
    clear();
    m_project = project;    
    m_program = program;        
    m_program->clear();    
    
    auto list = m_project->globalVariableList();
    for(int i = 0; i < list.size(); i++)
    {
        QString name = list[i];
        m_program->m_variables[name] = *m_project->globalVariable(name);
    }
    
    auto class_list = m_project->itemList("./",ProjectItem_class);
    for(int i = 0; i < class_list.size(); i++)
    {
        JZScriptClassItem *script = dynamic_cast<JZScriptClassItem*>(class_list[i]);        
        m_program->m_objectDefines << script->objectDefine();
                
        auto params = script->itemList(ProjectItem_param);
        Q_ASSERT(params.size() == 0 || params.size() == 1);
        if(params.size() == 1)
        {
            auto param = dynamic_cast<JZParamItem*>(params[0]);
//            m_program->m_binds[script->className()] = param->binds();
        }
    }

    auto bind_list = m_project->itemList("./", ProjectItem_scriptParamBinding);
    for(int i = 0; i < bind_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(bind_list[i]);
        if(!buildScriptFile(script))
            return false;
    }

    auto function_list = m_project->itemList("./", ProjectItem_scriptFunction);
    for (int i = 0; i < function_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(function_list[i]);
        if (!buildScriptFile(script))
            return false;        
    }

    auto flow_list = m_project->itemList("./", ProjectItem_scriptFlow);
    for(int i = 0; i < flow_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(flow_list[i]);
        if(!buildScriptFile(script))
            return false;
    }

    if(!link())
        return false;

    return true;
}

bool JZNodeBuilder::buildScriptFile(JZScriptItem *scriptFile)
{       
    QString path = scriptFile->itemPath();
    if(!m_scripts.contains(path))    
        m_scripts[path] = JZNodeScriptPtr(new JZNodeScript());
    
    auto classFile = m_project->getItemClass(scriptFile);
    JZNodeScript *script = m_scripts[path].data();
    if(classFile)
        script->className = classFile->className();

    LOGI(Log_Compiler, "build " + scriptFile->itemPath());
    JZNodeCompiler compiler;
    if(!compiler.build(scriptFile,script))
    {        
        m_error += compiler.error();
        LOGE(Log_Compiler, compiler.error());
        return false;
    }
    
    return true;
}

bool JZNodeBuilder::link()
{       
    std::sort(m_program->m_objectDefines.begin(),m_program->m_objectDefines.end(),
        [](const JZNodeObjectDefine &d1,const JZNodeObjectDefine &d2)->bool{
            return d1.id < d2.id;
        });

    m_program->m_scripts = m_scripts;          

    initGlobal();

    return true;
}
