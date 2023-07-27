#include "JZNodeBuilder.h"
#include "JZParamFile.h"
#include "JZUiFile.h"

JZNodeBuilder::JZNodeBuilder()
{

}

JZNodeBuilder::~JZNodeBuilder()
{

}

QList<Graph*> JZNodeBuilder::graphs(QString filename) const
{
    Q_ASSERT(m_scripts.contains(filename));
    auto &graph_list = m_scripts[filename]->graphs;
    QList<Graph*> result;
    for(int i = 0; i < graph_list.size(); i++)         
        result.push_back(graph_list[i].data());
    
    return result;    
}

QString JZNodeBuilder::error() const
{
    return m_error;
}

bool JZNodeBuilder::build(JZProject *project,JZNodeProgram *program)
{
    m_error.clear();        
    m_scripts.clear();
    m_project = project;    
    m_program = program;        
    m_program->clear();

    //删除无用编译结果
    auto it = m_scripts.begin();
    while(it != m_scripts.end())
    {
        if(m_project->getItem(it.key()) == nullptr)
            it = m_scripts.erase(it);
        else
            it++;
    }
    
    auto list = m_project->globalVariableList();
    for(int i = 0; i < list.size(); i++)
    {
        QString name = list[i];
        m_program->m_variables[name] = *m_project->globalVariableInfo(name);
    }

    auto root = m_project->root();
    auto library_list = root->itemList(ProjectItem_library);
    for(int i = 0; i < library_list.size(); i++)
    {
        JZScriptLibraryFile *script = dynamic_cast<JZScriptLibraryFile*>(library_list[i]);
        if(!buildLibraryFile(script))
            return false;
    }

    auto class_list = root->itemList(ProjectItem_class);
    for(int i = 0; i < class_list.size(); i++)
    {
        JZScriptClassFile *script = dynamic_cast<JZScriptClassFile*>(class_list[i]);
        if(!buildClassFile(script))
            return false;
        
        auto define = script->objectDefine();
        m_program->m_objectDefines << define;        
    }

    auto function_list = root->itemList(ProjectItem_scriptFunction);
    for(int i = 0; i < function_list.size(); i++)
    {
        JZScriptFile *script = dynamic_cast<JZScriptFile*>(function_list[i]);
        if(!buildScriptFile(script))
            return false;

        m_program->m_functionDefines << script->function();
    }

    auto bind_list = root->itemList(ProjectItem_scriptParamBinding);
    for(int i = 0; i < bind_list.size(); i++)
    {
        JZScriptFile *script = dynamic_cast<JZScriptFile*>(bind_list[i]);
        if(!buildScriptFile(script))
            return false;
    }

    auto flow_list = root->itemList(ProjectItem_scriptFlow);
    for(int i = 0; i < flow_list.size(); i++)
    {
        JZScriptFile *script = dynamic_cast<JZScriptFile*>(flow_list[i]);
        if(!buildScriptFile(script))
            return false;
    }

    if(!link())
        return false;

    return true;
}

bool JZNodeBuilder::buildScriptFile(JZScriptFile *scriptFile)
{   
    QString path = scriptFile->itemPath();
    if(!m_scripts.contains(path))    
        m_scripts[path] = JZNodeScriptPtr(new JZNodeScript());
    
    auto classFile = m_project->getClassFile(scriptFile);
    JZNodeScript *script = m_scripts[path].data();
    if(classFile)
        script->className = classFile->className();

    JZNodeCompiler compiler;
    if(!compiler.build(scriptFile,script))
    {
        m_error += compiler.error();
        return false;
    }
    
    return true;
}

bool JZNodeBuilder::buildLibraryFile(JZScriptLibraryFile *script)
{
    return true;
}

bool JZNodeBuilder::buildClassFile(JZScriptClassFile *classFile)
{
    auto flowList = classFile->itemList(ProjectItem_scriptFlow);
    Q_ASSERT(flowList.size() == 1);

    JZScriptFile *flow_script = dynamic_cast<JZScriptFile*>(flowList[0]);
    if(!buildScriptFile(flow_script))
        return false;

    auto funcList = classFile->itemList(ProjectItem_scriptFunction);
    for(int i = 0; i < funcList.size(); i++)
    {
        JZScriptFile *script = dynamic_cast<JZScriptFile*>(funcList[i]);
        if(!buildScriptFile(script))
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
    return true;
}
