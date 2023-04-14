#include "JZProject.h"
#include "JZNodeFactory.h"

// JZProject
JZProject::JZProject()
    :m_root(ProjectItem_root,true)
{
    m_nodeId = 0;    
}

JZProject::~JZProject()
{
}

void JZProject::clear()
{

}

JZProjectItem *JZProject::root()
{
    &m_root;
}

void JZProject::sort()
{
   /* std::sort(m_files.begin(),m_files.end()[]{
        return < 
    });*/
}

void JZProject::addFile(QString dir,JZProjectItem *item)
{
        
}

void JZProject::removeFile(QString filepath)
{
    JZProjectItem *item = getItem(filepath);
    auto parent = item->parent();
    parent->removeItem(item);
}

JZProjectItem *JZProject::getItem(QString path)
{
    QStringList path_list = path.split("/");
    JZProjectItem *folder = &m_root;
    for(int i = 0; i < path_list.size(); i++)
    {        
        folder = folder->getItem(path_list[i]);
    }
    return folder;
}

void JZProject::makeTree()
{
    for(int i = 0; i < m_files.size(); i++){
        JZProjectItem *dir = getItem(m_files[i]->path());
        dir->addItem(m_files[i].data());
    }
}