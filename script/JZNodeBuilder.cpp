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

bool JZNodeBuilder::buildItem(JZProjectItem *item)
{
    QVector<int> script_types = {ProjectItem_scriptFlow,ProjectItem_scriptParam,ProjectItem_scriptFunction};
    if(item->isFolder())
    {
        QList<JZProjectItem*> item_list = item->childs();
        for(int i = 0; i< item_list.size(); i++)
        {
            if(!buildItem(item_list[i]))
                return false;
        }
    }
    else
    {
        if(script_types.contains(item->itemType()))
        {
            JZScriptFile *script = (JZScriptFile *)item;
            return buildScriptFile(script);
        }
    }
    return true;
}

bool JZNodeBuilder::build(JZProject *project,JZNodeProgram *program)
{
    m_error.clear();        
    m_scripts.clear();
    m_project = project;    
    m_program = program;        

    //删除无用编译结果
    auto it = m_scripts.begin();
    while(it != m_scripts.end())
    {
        if(m_project->getItem(it.key()) == nullptr)
            it = m_scripts.erase(it);
        else
            it++;
    }
    
    auto list = m_project->variableList();
    for(int i = 0; i < list.size(); i++)
    {
        QString name = list[i];
        m_program->m_variables[name] = m_project->getVariable(name);
    }

    if(!buildItem(m_project->root()))
        return false;    

    if(!link())
        return false;

    return true;
}

bool JZNodeBuilder::buildScriptFile(JZScriptFile *scriptFile)
{   
    QString path = scriptFile->itemPath();
    if(!m_scripts.contains(path))    
        m_scripts[path] = JZNodeScriptPtr(new JZNodeScript());
    
    JZNodeScript *script = m_scripts[path].data();
    JZNodeCompiler compiler;
    if(!compiler.build(scriptFile,script))
    {
        m_error += compiler.error();
        return false;
    }
    
    return true;
}

bool JZNodeBuilder::link()
{   
    m_program->clear();
    m_program->m_scripts = m_scripts;            
    return true;
}
