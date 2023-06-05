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
    m_root.removeChlids();
    m_filepath.clear();
    m_variables.clear();

    m_root.setName(".");
    JZProjectItemFolder *ui = new JZProjectItemFolder();
    JZProjectItemFolder *data = new JZProjectItemFolder();
    JZProjectItemFolder *script_flow = new JZProjectItemFolder();
    JZProjectItemFolder *script_param = new JZProjectItemFolder();
    data->setName("变量定义");
    ui->setName("用户界面");
    script_param->setName("数据联动");
    script_flow->setName("程序流程");
    addItem("./",ui);
    addItem("./",data);
    addItem("./",script_param);
    addItem("./",script_flow);

    JZScriptFile *data_page = new JZScriptFile(ProjectItem_param);
    JZScriptFile *ui_page = new JZScriptFile(ProjectItem_ui);
    JZScriptFile *flow_page = new JZScriptFile(ProjectItem_scriptFlow);
    JZScriptFile *param_page = new JZScriptFile(ProjectItem_scriptParam);
    data_page->setName("变量");
    ui_page->setName("mainwindow.ui");
    param_page->setName("联动");
    flow_page->setName("main.jz");

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
    m_root.loadFromStream(s);
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
    m_root.saveToStream(s);
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

QString JZProject::mainScript()
{
    return "./程序流程/main.jz";
}

JZProjectItem *JZProject::root()
{
    return &m_root;
}

int JZProject::addItem(QString dir,JZProjectItem *item)
{    
    auto parent = getItem(dir);
    if(!parent)
        return -1;    
    
    Q_ASSERT(!item->project());
    item->setProject(this);
    parent->addItem(JZProjectItemPtr(item));
    item->parent()->sort();
    return item->parent()->indexOfItem(item);
}

void JZProject::removeItem(QString filepath)
{
    JZProjectItem *item = getItem(filepath);
    auto parent = item->parent();
    int index = parent->indexOfItem(item);
    parent->removeItem(index);
}

int JZProject::renameItem(JZProjectItem *item,QString newname)
{
    item->setName(newname);
    item->parent()->sort();
    return item->parent()->indexOfItem(item);
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

void JZProject::registObject(JZNodeObjectDefine def,QString super)
{
    JZNodeObjectManager::instance()->regist(def,super);
}

void JZProject::unregistObject(QString name)
{

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

void JZProject::addClassVariable(QString name,QString className)
{
    JZNodeObjectDelcare delcare;
    delcare.className = className;
    m_variables[name] = QVariant::fromValue(delcare);
}

QString JZProject::getClassVariable(QString name)
{
    return m_variables[name].value<JZNodeObjectDelcare>().className;
}

QStringList JZProject::variableList()
{
    return m_variables.keys();
}

QList<JZProjectItem *> JZProject::itemList(QString path,int type)
{
    QList<JZProjectItem *> list;
    itemList(getItem(path),type,list);
    return list;
}

void JZProject::itemList(JZProjectItem *item,int type,QList<JZProjectItem *> &list)
{
    if(item->isFolder())
    {
        auto chlid = item->childs();
        for(int i = 0; i < chlid.size(); i++)
            itemList(chlid[i],type,list);
    }
    else
    {
        if(item->itemType() == type)
            list << item;
    }
}

const FunctionDefine *JZProject::function(QString name)
{
    auto list = itemList("./",ProjectItem_scriptFunction);
    for(int i = 0; i < list.size(); i++)
    {
        JZScriptFile *file = (JZScriptFile*)list[i];
        if(file->function().name == name)
            return &file->function();
    }
    return nullptr;
}

void JZProject::saveToStream(QDataStream &s)
{

}

void JZProject::loadFromStream(QDataStream &s)
{    
}
