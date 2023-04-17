#include "JZNodeBuilder.h"


JZNodeBuilder::JZNodeBuilder()
{

}

JZNodeBuilder::~JZNodeBuilder()
{

}

const QList<Graph*> &JZNodeBuilder::graphs(QString filename) const
{
    Q_ASSERT(m_scripts.contains(filename));
    auto &graph_list = m_scripts[filename].graphs;
    QList<Graph*> result;
    for(int i = 0; i < graph_list.size(); i++)         
        result.push_back(graph_list[i].data());
    
    return result;    
}

bool JZNodeCompiler::build(JZProject *project,JZNodeProgram &result)
{
    m_error.clear();    
    m_project = project;    
    m_program = JZNodeProgram();

    QVector<int> script_types = {ProjectItem_scriptFlow,ProjectItem_scriptParam};

    QList<JZProjectItem*> file_list = project->items();
    for(int i = 0; i < file_list.size(); i++)
    {   
        if(file_list[i]->isFolder())
            continue;
    
        if(script_types.contains(file_list[i]->itemType()))
        {
            JZScriptFile *script = (JZScriptFile *)file_list[i];
            buildScriptFile(script);
        }
    }
        
    result = m_program;
    return true;
}

bool JZNodeCompiler::link()
{
    return true;
}