#include <QFile>
#include <QFileInfo>
#include "JZProject.h"
#include "JZNodeFactory.h"
#include "JZScriptFile.h"

// JZProject
JZProject::JZProject()    
{    
    init();
}

JZProject::~JZProject()
{
}

void JZProject::init()
{
    m_items.clear();
    m_root.removeChlids();
    m_filepath.clear();
    m_variables.clear();

    JZScriptFile *ui = new JZScriptFile(ProjectItem_ui,true);
    JZScriptFile *data = new JZScriptFile(ProjectItem_param,true);
    JZScriptFile *script_flow = new JZScriptFile(ProjectItem_scriptFlow,true);
    JZScriptFile *script_param = new JZScriptFile(ProjectItem_scriptParam,true);
    data->setName("变量定义");
    ui->setName("用户界面");
    script_param->setName("数据联动");
    script_flow->setName("程序流程");
    addItem("./",ui);
    addItem("./",data);
    addItem("./",script_param);
    addItem("./",script_flow);

    JZScriptFile *data_page = new JZScriptFile(ProjectItem_param,false);
    JZScriptFile *ui_page = new JZScriptFile(ProjectItem_ui,false);
    JZScriptFile *flow_page = new JZScriptFile(ProjectItem_scriptFlow,false);
    JZScriptFile *param_page = new JZScriptFile(ProjectItem_scriptParam,false);
    data_page->setName("变量");
    ui_page->setName("界面");
    param_page->setName("联动");
    flow_page->setName("流程");

    addItem("./变量定义",data_page);
    addItem("./用户界面",ui_page);
    addItem("./数据联动",param_page);
    addItem("./程序流程",flow_page);
}

bool JZProject::open(QString filepath)
{
    QFile file(filepath);
    if(!file.open(QFile::ReadOnly))
        return false;

    QDataStream s(&file);    
    for(int i = 0; i < m_items.size(); i++)
        m_items[i]->loadFromStream(s);
    file.close();    

    return true;
}

bool JZProject::save()
{    
    return saveAs(m_filepath);    
}

bool JZProject::saveAs(QString filepath)
{   
    m_filepath = filepath;    

    QFile file(m_filepath);
    if(!file.open(QFile::WriteOnly | QFile::Truncate))
        return false;

    QDataStream s(&file);
    for(int i = 0; i < m_items.size(); i++)
    {
        s << m_items[i]->itemType();
        m_items[i]->saveToStream(s);
    }
    file.close();    
    return true;
}

QString JZProject::name()
{
    if(m_filepath.isEmpty())
        return "untitled";

    QFileInfo info(m_filepath);
    return info.baseName();
}

JZProjectItem *JZProject::root()
{
    return &m_root;
}

void JZProject::sort()
{
   /* std::sort(m_items.begin(),m_items.end()[]{
        return < 
    });*/
}

int JZProject::addItem(QString dir,JZProjectItem *item)
{    
    auto parent = getItem(dir);
    if(!parent)
        return -1;    

    m_items.push_back(JZProjectItemPtr(item));
    parent->addItem(item);
    item->parent()->sort();
    return item->parent()->indexOfItem(item);
}

void JZProject::removeItem(QString filepath)
{
    JZProjectItem *item = getItem(filepath);
    auto parent = item->parent();
    parent->removeItem(item);

    for(int i = 0; i < m_items.size(); i++)
    {
        if(m_items[i].data() == item)
        {
            m_items.removeAt(i);
            break;
        }
    }
}

int JZProject::renameItem(JZProjectItem *item,QString newname)
{
    item->setName(newname);
    item->parent()->sort();
    return item->parent()->indexOfItem(item);
}

QList<JZProjectItem*> JZProject::items()
{
    QList<JZProjectItem*> list;
    for(int i = 0; i < m_items.size(); i++)
    {
        list.push_back(m_items[i].data());
    }
    return list;
}

JZProjectItem *JZProject::getItem(QString path)
{
    if(path == "." || path == "./")
        return &m_root;

    if(!path.startsWith("./"))
        path += "./";        
    QStringList path_list = path.split("/",Qt::KeepEmptyParts);
    JZProjectItem *folder = &m_root;
    for(int i = 1; i < path_list.size(); i++)
    {        
        folder = folder->getItem(path_list[i]);
        if(!folder)
            return nullptr;
    }
    return folder;
}

void JZProject::makeTree()
{
    for(int i = 0; i < m_items.size(); i++){
        JZProjectItem *dir = getItem(m_items[i]->path());
        dir->addItem(m_items[i].data());
    }
}

void JZProject::addVariable(QString name,QVariant value)
{
    m_variables[name] = value;
}

void JZProject::removeVariable(QString name)
{
    m_variables.remove(name);
}

void JZProject::setVariable(QString name,QVariant value)
{
    m_variables[name] = value;
}

QVariant JZProject::getVariable(QString name)
{
    return m_variables[name];
}

QStringList JZProject::variableList()
{
    return m_variables.keys();
}

void JZProject::saveToStream(QDataStream &s)
{

}

void JZProject::loadFromStream(QDataStream &s)
{
    makeTree();
}
