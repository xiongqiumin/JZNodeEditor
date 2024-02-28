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
#include "JZProjectTemplate.h"

// JZProject
JZProject::JZProject()    
{            
    clear();
}

JZProject::~JZProject()
{
}

bool JZProject::isNull() const
{
    return m_filepath.isEmpty();
}

void JZProject::clear()
{
    m_blockRegist = false;
    m_windowSystem = false;
    m_filepath.clear();
    m_root.removeChlids();
    m_root.setName(".");    

    JZNodeFunctionManager::instance()->clearUserReigst();
    JZNodeObjectManager::instance()->clearUserReigst();
}

QByteArray JZProject::magic()
{
    QString magic = "1234567";

    QByteArray result;
    QDataStream s(&result, QIODevice::WriteOnly);
    s << QString(magic);

    return result;
}

bool JZProject::newProject(QString path,QString name, QString temp)
{
    if (!JZProjectTemplate::instance()->initProject(path, name, temp))
        return false;

    return open(path + "/" + name + "/" + name + ".jzproj");
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

    QStringList file_list;
    auto &pro_s = file.stream();
    pro_s["filelist"] >> file_list;

    m_blockRegist = true;        
    m_filepath = filepath;

    QString dir = QFileInfo(m_filepath).path();
    for (int i = 0; i < file_list.size(); i++)
    {        
        QString sub_path;
        if(QDir::isAbsolutePath(file_list[i]))
            sub_path = file_list[i];
        else
            sub_path = dir + "/" + file_list[i];
        
        addFile(sub_path);        
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

    auto item_list = m_root.itemList({ProjectItem_scriptFile, ProjectItem_ui});
    QStringList file_list;
    for (int i = 0; i < item_list.size(); i++)    
        file_list << item_list[i]->itemPath();    
    
    JZProjectFile file;
    auto &pro_s = file.stream();
    pro_s["filelist"] << file_list;

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
    s << breakPoints();
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
        QMap<QString, QList<int>> breakPoints;
        s >> breakPoints;        

        auto it = breakPoints.begin();
        while (it != breakPoints.end())
        {
            auto item = getItem(it.key());
            if (item)
                m_breakPoints[item] = it.value();            
            it++;
        }

        file.close();
    }
}

QString JZProject::name()
{    
    QFileInfo info(m_filepath);
    return info.baseName();
}

void JZProject::setFilePath(QString path)
{
    m_filepath = path;
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
    JZScriptFile *file = (JZScriptFile*)getItem("./main.jz");
    return file->flow("main");
}

JZParamItem *JZProject::globalDefine()
{
    JZScriptFile *file = (JZScriptFile*)getItem("./main.jz");
    return file->paramDefine("global");
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
    auto class_file = getItemClass(item);
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

bool JZProject::saveItem(JZProjectItem *item)
{
    QList<JZProjectItem*> items;
    items << item;
    return saveItems(items);
}

bool JZProject::saveItems(QList<JZProjectItem*> items)
{
    QMap<JZProjectItem*,QList<JZProjectItem*>> file_item;
    for (int i = 0; i < items.size(); i++)
    {
        auto item = getItemFile(items[i]);
        if(item)            
            file_item[item].push_back(items[i]);
    }

    auto it = file_item.begin();
    while (it != file_item.end())
    {
        auto file = it.key();
        QString file_path = file->itemPath();
        if (file_path.startsWith("./"))
            file_path = path() + "/" + file_path.mid(2);

        if (file->itemType() == ProjectItem_ui)
        {
            JZUiFile *ui_file = dynamic_cast<JZUiFile*>(file);
            if (!ui_file->save(file_path))
                return false;
        }
        else if (file->itemType() == ProjectItem_scriptFile)
        {
            JZScriptFile *script_file = dynamic_cast<JZScriptFile*>(file);
            
            bool ret = false;
            if (it.value().contains(script_file))
                ret = script_file->save(file_path);
            else
                ret = script_file->save(file_path, it.value());

            if (!ret)
                return false;
        }

        it++;
    }

    return true;
}

bool JZProject::saveAllItem()
{
    auto items = itemList("./", ProjectItem_any);
    return saveItems(items);
}

bool JZProject::loadItem(JZProjectItem *item)
{
    return true;
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

JZProjectItem *JZProject::addFile(QString filepath)
{    
    JZProjectItem *item = nullptr;
    if (!QFile::exists(filepath))
        return false;

    filepath = QFileInfo(filepath).canonicalFilePath();

    QString sub_dir = QFileInfo(filepath).path();    
    if (sub_dir.startsWith(path()))
        sub_dir = "." + sub_dir.mid(path().size());

    QString fileName = QFileInfo(filepath).fileName();
    QString ext = QFileInfo(filepath).suffix();    
    if (ext == "jz")
    {
        JZScriptFile *script_file = new JZScriptFile();
        script_file->setName(fileName);
        addItem(sub_dir, script_file);
        script_file->load(filepath);
        item = script_file;
    }
    else if (ext == "ui")
    {
        JZUiFile *ui_file = new JZUiFile();
        ui_file->setName(fileName);
        addItem(sub_dir, ui_file);
        ui_file->load(filepath);        
        item = ui_file;
    }
    return item;
}

void JZProject::removeFile(QString path)
{
    removeItem(path);
}

void JZProject::renameFile(QString oldPath, QString newPath)
{
    auto item = getItem(oldPath);
    QString name = QFileInfo(newPath).fileName();
    item->setName(name);
}

JZScriptClassItem *JZProject::getClass(QString class_name)
{
    auto list = itemList("./", ProjectItem_class);
    for (int i = 0; i < list.size(); i++)
    {
        if(list[i]->name() == class_name)
            return dynamic_cast<JZScriptClassItem*>(list[i]);
    }
    return nullptr;
}

JZScriptClassItem *JZProject::getItemClass(JZProjectItem *item)
{    
    while (item)
    {
        if (item->itemType() == ProjectItem_class)
            return (JZScriptClassItem*)item;

        item = item->parent();
    }
    return nullptr;
}

JZProjectItem *JZProject::getItemFile(JZProjectItem *item)
{
    while (item)
    {
        if (item->itemType() == ProjectItem_ui
            || item->itemType() == ProjectItem_scriptFile)
            return (JZScriptClassItem*)item;

        item = item->parent();
    }
    return nullptr;
}

const JZParamDefine *JZProject::globalVariable(QString name)
{   
    return globalDefine()->variable(name);
}

QStringList JZProject::globalVariableList()
{    
    return globalDefine()->variableList();
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
    auto item = getItem(file);
    if (!m_breakPoints.contains(item))
        return false;
    
    return m_breakPoints[item].contains(id);
}

void JZProject::addBreakPoint(QString file,int id)
{
    if(hasBreakPoint(file,id))
        return;    

    auto item = getItem(file);
    m_breakPoints[item].push_back(id);
}

void JZProject::removeBreakPoint(QString file,int id)
{    
    if (!hasBreakPoint(file, id))
        return;

    auto item = getItem(file);
    m_breakPoints[item].removeAll(id);
    if (m_breakPoints[item].size() == 0)
        m_breakPoints.remove(item);
}

QMap<QString, QList<int>> JZProject::breakPoints()
{
    QMap<QString, QList<int>> result;
    auto it = m_breakPoints.begin();
    while (it != m_breakPoints.end())
    {
        result[it.key()->itemPath()] = it.value();
        it++;
    }
    return result;
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

    auto class_file = getItemClass(item);
    if (class_file)
    {
        //起到声明作用
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

        //覆盖注册
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