#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "JZProject.h"
#include "JZNodeFactory.h"
#include "JZScriptItem.h"
#include "JZUiFile.h"
#include "JZEvent.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeFunction.h"
#include "JZNodeEvent.h"
#include "JZNodeValue.h"

// JZProject
JZProject::JZProject()    
{        
    m_blockRegist = false;
    m_windowSystem = false;
}

JZProject::~JZProject()
{
    close();
}

bool JZProject::isVaild()
{
    return !m_filepath.isEmpty();
}

void JZProject::clear()
{
    m_filepath.clear();
    m_root.removeChlids();
    m_root.setName(".");    
}

QByteArray JZProject::magic()
{
    QString magic = "1234567";

    QByteArray result;
    QDataStream s(&result, QIODevice::WriteOnly);
    s << QString(magic);

    return result;
}

void JZProject::init()
{        
    clear();

    JZScriptItem *main_flow = new JZScriptItem(ProjectItem_scriptFlow);
    main_flow->setName("main.jz");
    addItem("",main_flow);

    JZParamItem *param_page = new JZParamItem();
    param_page->setName("global.def");
    addItem("",param_page);

    JZNodeEvent *start = new JZNodeStartEvent();    
    main_flow->addNode(JZNodePtr(start));

    saveItem(main_flow);
    m_windowSystem = false;
}

void JZProject::initUi()
{   
    init();

    m_windowSystem = true;
    m_blockRegist = true;    

    auto func_inst = JZNodeFunctionManager::instance();    
    auto script_file = createScript("mainwindow");
    script_file->addClass("mainwindow");

    m_blockRegist = false;
    registType();

    auto main_script = (JZScriptItem *)getItem("./main.jz");
    auto param_def = (JZParamItem *)getItem("./global.def");
    param_def->addVariable("mainwindow",JZClassId("mainwindow"));

    JZNodeSetParam *set_param = new JZNodeSetParam();    
    JZNodeCreate *create = new JZNodeCreate();    
    JZNodeFunction *func_show = new JZNodeFunction();
    int node_start = main_script->nodeList()[0];
    int node_create = main_script->addNode(JZNodePtr(create));
    int node_set = main_script->addNode(JZNodePtr(set_param));
    int node_show = main_script->addNode(JZNodePtr(func_show));

    set_param->setVariable("mainwindow");
    create->setClassName("mainwindow");
    func_show->setFunction(func_inst->function("Widget.show"));

    JZNode *start = main_script->getNode(0);   
    main_script->addConnect(start->flowOutGemo(),create->flowInGemo());

    main_script->addConnect(create->flowOutGemo(),set_param->flowInGemo());
    main_script->addConnect(create->paramOutGemo(0),set_param->paramInGemo(0));

    main_script->addConnect(set_param->flowOutGemo(0),func_show->flowInGemo());
    main_script->addConnect(set_param->paramOutGemo(0),func_show->paramInGemo(0));

    main_script->setNodePos(node_start, QPointF(0, 0));
    main_script->setNodePos(node_create, QPointF(150, 0));
    main_script->setNodePos(node_set, QPointF(480, 0));
    main_script->setNodePos(node_show, QPointF(730, 0));

    saveItem(param_def);
    saveItem(main_script);       
}

void JZProject::initTest()
{
    m_blockRegist = true;

    clear();

    JZScriptFile *file = new JZScriptFile();
    file->setName("main.jz");
    addItem("./", file);

    JZScriptItem *main_flow = file->addFlow("main");
    JZParamDefine *param = file->addParamDefine("global");

    JZNodeEvent *start = new JZNodeStartEvent();
    main_flow->addNode(JZNodePtr(start));

    saveItem(main_flow);
    m_windowSystem = false;

    m_blockRegist = false;
    registType();
}

void JZProject::initConsole()
{
    m_blockRegist = true;
    
    init();
    
    m_blockRegist = false;
    registType();
}

void JZProject::registType()
{
    JZNodeFunctionManager::instance()->clearUserReigst();
    JZNodeObjectManager::instance()->clearUserReigst();

    // regist type
    QList<JZProjectItem *> function_list = itemList("./",ProjectItem_scriptFunction);
    for (int i = 0; i < function_list.size(); i++)
    {
        if(!function_list[i]->getClassFile())
            regist(function_list[i]);
    }

    QList<JZProjectItem *> class_list = itemList("./",ProjectItem_class);
    for(int i = 0; i < class_list.size(); i++)    
        regist(class_list[i]);    
}

bool JZProject::open(QString filepath)
{
    clear();
    JZProjectFile file;    
    if (!file.load(filepath))
    {
        m_error = file.error();
        return false;
    }

    auto &pro_s = file.stream();
    pro_s["filelist"] >> m_fileList;

    m_blockRegist = true;        
    m_filepath = filepath;

    QString dir = QFileInfo(m_filepath).filePath();
    for (int i = 0; i < m_fileList.size(); i++)
    {
        QString sub_path;
        if(QDir::isAbsolutePath(m_fileList[i]))
            sub_path = m_fileList[i];
        else
            sub_path = dir + "/" + m_fileList[i];
                                
        if (!QFile::exists(sub_path))
            return false;

        QString ext = QFileInfo(sub_path).suffix();
        QString sub_dir = QFileInfo(sub_path).path();
        if (ext == "jz")
        {
            JZProjectFile sub_file;
            if (!sub_file.load(sub_path))
                return false;

            auto s = sub_file.stream();            

            JZScriptFile *class_file = new JZScriptFile();
            class_file->loadFromStream(s);
            addItem(sub_dir, class_file);
        }
        else if (ext == "ui")
        {
            QFile file(sub_path);
            if (!file.open(QFile::WriteOnly | QFile::Text))
                return false;

            JZUiFile *ui_file = new JZUiFile();
            addItem(sub_dir, ui_file);
        }
    }    
    m_blockRegist = false;

    registType();

    auto script_list = itemList("./", ProjectItem_any);
    for (int i = 0; i < script_list.size(); i++)
    {        
        if (JZProjectItemIsScript(script_list[i]))
        {            
            auto item = dynamic_cast<JZScriptItem*>(script_list[i]);
            item->loadFinish();
        }
    }    

    loadCache();
    return true;
}

void JZProject::close()
{
    saveCache();
    clear();    
}

bool JZProject::save()
{    
    if (m_filepath.isEmpty())
        return false;
    
    JZProjectFile file;
    auto &pro_s = file.stream();
    pro_s["filelist"] << m_fileList;

    return file.save(m_filepath);        
}

QString JZProject::error()
{
    return m_error;
}

void JZProject::saveCache()
{    
    QString cache = m_filepath + ".data";
    QFile file(cache);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return;

    QDataStream s(&file);
    s << magic();
    s << m_breakPoints;
    file.close();
}

void JZProject::loadCache()
{
    QString cache = m_filepath + ".data";
    QFile file(cache);
    if (!file.open(QFile::ReadOnly))
        return;

    QByteArray pre_magic;
    QDataStream s(&file);
    s >> pre_magic;
    if (pre_magic == magic())
    {
        s >> m_breakPoints;
        file.close();
    }
}

QString JZProject::name()
{    
    QFileInfo info(m_filepath);
    return info.baseName();
}

QString JZProject::filePath()
{
    return m_filepath;
}

QString JZProject::path()
{
    QFileInfo info(m_filepath);
    return info.path();
}

QString JZProject::mainScriptPath()
{
    return "./main.jz";
}

JZScriptItem *JZProject::mainScript()
{
    return dynamic_cast<JZScriptItem*>(getItem("./main.jz"));
}

JZParamItem *JZProject::globalDefine()
{
    return dynamic_cast<JZParamItem*>(getItem("./global.def"));
}

JZProjectItem *JZProject::root()
{
    return &m_root;
}

bool JZProject::addItem(QString dir, JZProjectItem *item)
{
    auto parent = getItem(dir);
    if (!parent)
        return false;

    Q_ASSERT(!item->project());
    Q_ASSERT(!item->name().isEmpty());
    item->setProject(this);
    parent->addItem(JZProjectItemPtr(item));
    item->parent()->sort();

    regist(item);
    return true;
}

void JZProject::removeItem(QString filepath)
{
    JZProjectItem *item = getItem(filepath);
    Q_ASSERT(item);

    bool replace_class = false;
    auto class_file = getClass(item);
    if (class_file)
    {
        if (item->itemType() == ProjectItem_class)
            JZNodeObjectManager::instance()->unregist(class_file->classType());
        else
            replace_class = true;
    }
    else
    {
        if (item->itemType() == ProjectItem_scriptFunction)
            JZNodeFunctionManager::instance()->unregistFunction(item->name());
    }

    auto parent = item->parent();
    int index = parent->indexOfItem(item);
    parent->removeItem(index);

    if (replace_class)
        JZNodeObjectManager::instance()->replace(class_file->objectDefine());
}

void JZProject::renameItem(JZProjectItem *item, QString newname)
{
    QString newPath = item->path() + "/" + newname;
    QString oldPath = item->itemPath();
    item->setName(newname);
    item->parent()->sort();

    oldPath += "/";
}

JZProjectItem *JZProject::getItem(QString path)
{    
    if(path.isEmpty() || path == "." || path == "./")
        return &m_root;
    if(!path.startsWith("./"))
        path = "./" + path;

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

JZProjectItem *JZProject::addFile(QString dir)
{
    return nullptr;
}

void JZProject::removeFile(QString path)
{

}

void JZProject::renameFile(QString oldPath, QString newPath)
{

}

JZScriptFile *JZProject::createScript(QString path)
{
    JZScriptFile *script = nullptr;
    return script;
}

JZScriptClassItem *JZProject::getClass(JZProjectItem *item)
{    
    while (item)
    {
        if (item->itemType() == ProjectItem_class)
            return (JZScriptClassItem*)item;

        item = item->parent();
    }
    return nullptr;
}

JZParamDefine *JZProject::globalVariableInfo(QString name)
{   
    QList<JZProjectItem*> list = paramDefineList();
    for(int i = 0; i < list.size(); i++)
    {        
        JZParamItem *file = dynamic_cast<JZParamItem*>(list[i]);
        JZParamDefine *def = file->getVariable(name);
        if (def)
            return def;
    }
    return nullptr;
}

QStringList JZProject::globalVariableList()
{    
    QStringList result;
    QList<JZProjectItem*> list = paramDefineList();
    for(int i = 0; i < list.size(); i++)
    {
        JZParamItem *file = dynamic_cast<JZParamItem*>(list[i]);
        result << file->variableList();
    }
    return result;
}

QList<JZProjectItem *> JZProject::itemList(QString path,int type)
{    
    auto item = getItem(path);
    Q_ASSERT(item);
    return item->itemList(type);
}

QList<JZProjectItem *> JZProject::paramDefineList()
{
    QList<JZProjectItem *> result;
    QList<JZProjectItem *> tmp = itemList("./", ProjectItem_param);
    for (int i = 0; i < tmp.size(); i++)
    {
        if (domain(tmp[i]).isEmpty())
            result.push_back(tmp[i]);
    }
    return result;
}

const FunctionDefine *JZProject::function(QString name)
{
    auto list = itemList("./",ProjectItem_scriptFunction);
    for(int i = 0; i < list.size(); i++)
    {
        JZScriptItem *file = (JZScriptItem*)list[i];
        if(file->function().name == name)
            return &file->function();
    }
    return nullptr;
}

QStringList JZProject::functionList()
{
    QStringList ret;

    auto list = itemList("./", ProjectItem_scriptFunction);
    for (int i = 0; i < list.size(); i++)
    {
        JZScriptItem *file = (JZScriptItem*)list[i];
        if (file->function().className.isEmpty())
            ret << file->function().fullName();
    }
    return ret;
}

QString JZProject::dir(const QString &filepath)
{
    int index = filepath.lastIndexOf("/");
    return filepath.left(index);
}

bool JZProject::hasBreakPoint(QString file,int id)
{
    return false;
}

void JZProject::addBreakPoint(QString file,int id)
{
    if(hasBreakPoint(file,id))
        return;    
}

void JZProject::removeBreakPoint(QString file,int id)
{    
}

QMap<QString, QList<int>> JZProject::breakPoints()
{
    return m_breakPoints;
}

QString JZProject::domain(JZProjectItem *item)
{
    QString result;
    while(item)
    {
        if (item->itemType() == ProjectItem_class)
        {
            if (!result.isEmpty())
                result += "::";
            result = item->name() + result;
        }
        item = item->parent();
    }
    return result;
}

void JZProject::regist(JZProjectItem *item)
{
    if (m_blockRegist)
        return;

    auto class_file = getClass(item);
    if (class_file)
    {
        JZNodeObjectDefine base;
        base.className = class_file->name();
        base.superName = class_file->superClass();
        base.id = class_file->classType();                
        if (base.id == -1)
        {
            int id = JZNodeObjectManager::instance()->regist(base);
            class_file->setClassType(id);
        }
        else
        {
            JZNodeObjectManager::instance()->replace(base);            
        }

        auto new_def = class_file->objectDefine();                        
        JZNodeObjectManager::instance()->replace(new_def);
    }
    else
    {
        if (item->itemType() == ProjectItem_scriptFunction)
        {
            JZScriptItem* func = dynamic_cast<JZScriptItem*>(item);            
            JZNodeFunctionManager::instance()->registFunction(func->function());
        }
    }
    emit sigFileChanged();
}