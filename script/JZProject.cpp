#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "JZProject.h"
#include "JZNodeFactory.h"
#include "JZScriptItem.h"
#include "JZUiItem.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeFunction.h"
#include "JZNodeEvent.h"
#include "JZNodeValue.h"
#include "JZProjectTemplate.h"
#include "JZContainer.h"
#include "JZNodeProgram.h"
#include "JZModule.h"
#include "JZProjectFile.h"

//BreakPoint
BreakPoint::BreakPoint()
{
    type = none;
    nodeId = -1;
}

void operator<<(QDataStream &s, const BreakPoint &param)
{
    s << param.scriptItemPath;
    s << param.nodeId;
    s << param.type;
}

void operator>>(QDataStream &s, BreakPoint &param)
{
    s >> param.scriptItemPath;
    s >> param.nodeId;
    s >> param.type;
}

//JZProjectTempGuard
JZProjectTempGuard::JZProjectTempGuard(JZProject* project, JZProjectItem* item, AfterOpertaor op)
    :JZProjectTempGuard(project, QList<JZProjectItem*>{item}, op)
{
}

JZProjectTempGuard::JZProjectTempGuard(JZProject* project, QList<JZProjectItem*> items, AfterOpertaor op)
{
    m_project = project;
    m_after = op;
    m_items = items;
    for (int i = 0; i < m_items.size(); i++)
    {
        auto item = m_items[i];
        project->addTmp(item);
        if (item->itemType() == ProjectItem_scriptItem)
        {
            JZScriptItem* script_item = dynamic_cast<JZScriptItem*>(item);
            script_item->loadFinish();
        }
    }
}

JZProjectTempGuard::~JZProjectTempGuard()
{
    for (int i = 0; i < m_items.size(); i++)
    {
        auto item = m_items[i];
        if (m_after == TakeItem)
            m_project->takeTmp(item);
        else
            m_project->removeTmp(item);
    }
}

void JZProjectTempGuard::setClass(QString className)
{
    for (int i = 0; i < m_items.size(); i++)
        m_project->setTmpClass(m_items[i], className);
}

//JZProject
JZProject::JZProject()    
{           
    m_root.setName(".");
    m_tmp.setName("/tmp");

    m_root.setRootProject(this);
    m_tmp.setRootProject(this);
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
    m_root.clearChlids();
    m_root.setName(".");

    m_tmp.clearChlids();
    m_tmp.setName("/tmp");
    
    m_blockRegist = false;    
    m_isSaveCache = false;
    m_filepath.clear();            
    m_saveCache.clear();
    m_breakPoints.clear();

    registType();
}

void JZProject::copyTo(JZProject *other) const
{
    other->clear();
    other->m_root.fromBuffer(m_root.toBuffer());
    other->m_tmp.fromBuffer(m_tmp.toBuffer());
    other->registType();
    other->loadFinish();
}

JZScriptEnvironment *JZProject::environment()
{
    return &m_env;
}

const JZScriptEnvironment *JZProject::environment() const
{
    return &m_env;
}

void JZProject::registType()
{
    unregistType();

    JZNodeTypeMeta meta;
    QList<JZProjectItem *> class_list = itemList("./",ProjectItem_class);
    for(int i = 0; i < class_list.size(); i++)    
    {
        auto class_item = dynamic_cast<JZScriptClassItem*>(class_list[i]);
        meta.objectList << class_item->objectDefine();
    }

    QList<JZProjectItem *> function_list = itemList("./",ProjectItem_scriptItem);
    for (int i = 0; i < function_list.size(); i++)
    {
        if(!function_list[i]->getClassItem())
        {
            auto script_item = dynamic_cast<JZScriptItem*>(function_list[i]);
            meta.functionList << script_item->function();
        }
    }
    m_env.registType(meta);    
}

void JZProject::unregistType()
{
    m_env.unregistType();
}

QString JZProject::absoluteFilePath(QString path) const
{    
    if (m_filepath.isEmpty())
        return path;
    else
    {
        QDir dir(this->path());            
        return dir.absoluteFilePath(path);
    }    
}

bool JZProject::open(QString filepath)
{
    clear();
    JZProjectFile file;    
    if (!file.openLoad(filepath))
    {
        m_error = file.error();
        return false;
    }

    QStringList file_list;
    auto &pro_s = file.stream();    
    pro_s >> file_list;
    file.close();

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
    loadCache();
    loadFinish();        
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
    if(!file.openSave(m_filepath))
    {
        m_error = file.error();
        return false;
    }

    auto item_list = m_root.itemList({ProjectItem_scriptFile, ProjectItem_ui});
    QStringList file_list;
    for (int i = 0; i < item_list.size(); i++)    
        file_list << item_list[i]->itemPath();    

    auto &pro_s = file.stream();    
    pro_s << file_list;

    file.close();
    return true;
}

bool JZProject::saveAs(QString filepath)   //只保存工程自身，不保存项目文
{
    m_filepath = filepath;
    return save();
}

void JZProject::loadFinish()
{
    auto script_list = itemList("./", ProjectItem_any);
    for (int i = 0; i < script_list.size(); i++)
    {
        if (JZProjectItemIsScript(script_list[i]))
        {
            auto item = dynamic_cast<JZScriptItem*>(script_list[i]);
            item->loadFinish();
        }
    }
}

void JZProject::addTmp(JZProjectItem *item)
{    
    if (item->name().isEmpty())
    {
        int idx = m_tmp.childCount();
        while (true)
        {
            QString name = "tmp" + QString::number(idx);
            if (!m_tmp.getItem(name))
            {
                item->setName(name);
                break;
            }
            idx++;
        }
    }
    addItem("/tmp", item);
}

void JZProject::setTmpClass(JZProjectItem* item, QString className)
{
    m_tmpClass[item] = className;
}

void JZProject::removeTmp(JZProjectItem *item)
{    
    removeItem(item->itemPath());
    m_tmpClass.remove(item);
}

void JZProject::takeTmp(JZProjectItem *item)
{
    m_tmp.takeItem(item);
    m_tmpClass.remove(item);
}

bool JZProject::isTmp(JZProjectItem *item)
{
    return item->itemPath().startsWith("/tmp");
}

JZScriptClassItem* JZProject::tmpClassItem(JZProjectItem* item)
{
    if (!m_tmpClass.contains(item))
        return nullptr;

    QString class_name = m_tmpClass[item];
    return getClass(class_name);
}

bool JZProject::isFile(JZProjectItem *item)
{
    if(item->itemType() == ProjectItem_scriptFile)
        return true;
    
    return false;
}

void JZProject::saveTransaction()
{
    m_isSaveCache = true;
}

void JZProject::saveCommit()
{
    m_isSaveCache = false;
    saveItems(m_saveCache);
    m_saveCache.clear();
}

QString JZProject::error() const
{
    return m_error;
}

void JZProject::saveCache()
{    
    if(m_filepath.isEmpty())
        return;

    QString cache = m_filepath + ".data";
    QFile file(cache);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return;

    QDataStream s(&file);
    s << ProjectMagic();
    s << breakPoints();
    file.close();
}

void JZProject::loadCache()
{
    if(m_filepath.isEmpty())
        return;
    
    QString cache = m_filepath + ".data";
    QFile file(cache);
    if (!file.open(QFile::ReadOnly))
        return;

    QByteArray pre_magic;
    QDataStream s(&file);
    s >> pre_magic;
    if (pre_magic == ProjectMagic())
    {
        QList<BreakPoint> breakPoints;
        s >> breakPoints;        

        for(int i = 0; i < breakPoints.size(); i++)
        {
            auto item = getItem(breakPoints[i].scriptItemPath);
            if (item)
                m_breakPoints[item] << breakPoints[i];
        }

        file.close();
    }
}

QString JZProject::name() const
{    
    QFileInfo info(m_filepath);
    return info.baseName();
}

void JZProject::setFilePath(QString path)
{
    m_filepath = path;
}

QString JZProject::filePath() const
{
    return m_filepath;
}

QString JZProject::path() const
{
    QFileInfo info(m_filepath);
    return info.path();
}

JZScriptFile *JZProject::mainFile()
{
    JZScriptFile *file = dynamic_cast<JZScriptFile*>(getItem(mainFilePath()));
    return file;
}

QString JZProject::mainFilePath()
{
    return "./main.jz";
}

QString JZProject::mainFunctionPath()
{
    return mainFunction()->itemPath();
}

JZScriptItem *JZProject::mainFunction()
{
    return mainFile()->getFunction("main");
}

JZParamItem *JZProject::globalDefine()
{
    return mainFile()->paramDefine("global");
}

JZProjectItem *JZProject::root()
{
    return &m_root;
}

bool JZProject::addItem(QString dir_path, JZProjectItem *item)
{
    if (!m_filepath.isEmpty() && QDir::isAbsolutePath(dir_path))
    {
        QDir dir(path());
        QString relative = dir.relativeFilePath(dir_path);
        if (relative == "." || relative.startsWith("./"))
        {
            dir_path = relative;
        }
    }

    auto parent = getItem(dir_path);
    if (!parent)
        return false;
    if (getItem(dir_path + "/" + item->name()))
        return false;

    Q_ASSERT(!item->project());
    Q_ASSERT(!item->name().isEmpty());    
    parent->addItem(item);

    onItemChanged(item);
    return true;
}

void JZProject::removeItem(QString filepath)
{
    JZProjectItem *item = getItem(filepath);
    Q_ASSERT(item);

    QList<JZScriptClassItem*> replace_list;    
    JZScriptClassItem *class_file = getItemClass(item);
    if (class_file)
    {
        if (item->itemType() == ProjectItem_class)
            m_env.objectManager()->unregist(class_file->classType());
        else
            replace_list << class_file;
    }
    else
    {        
        if (item->itemType() == ProjectItem_scriptItem)
            m_env.functionManager()->unregistFunction(item->name());
        else if (item->itemType() == ProjectItem_ui)
        {
            auto class_list = itemList("./", ProjectItem_class);
            replace_list << item->getClassItem();
        }
    }    

    auto file_item = getItemFile(item);
    bool is_file = (file_item == item);
    if(is_file)
    {
        QString item_path = path() + "/" + filepath;
        QFile::remove(item_path);
        save();
    }

    auto parent = item->parent();
    parent->removeItem(item);

    for(int i = 0; i < replace_list.size(); i++)
        m_env.objectManager()->replace(replace_list[i]->objectDefine());
    
    if(!is_file && file_item)
        saveItem(file_item);            
}

bool JZProject::saveItem(JZProjectItem *item)
{
    QList<JZProjectItem*> items;
    items << item;
    return saveItems(items);
}

bool JZProject::saveItems(QList<JZProjectItem*> items)
{
    if(m_isSaveCache)
    {
        m_saveCache.append(items);
        return true;
    }

    QMap<JZProjectItem*,QList<JZProjectItem*>> file_item;
    for (int i = 0; i < items.size(); i++)   
    {        
        Q_ASSERT(items[i]->project() == this);
        auto file = getItemFile(items[i]);        
        if (file)        
            file_item[file].push_back(items[i]);        
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
            JZUiItem *ui_file = dynamic_cast<JZUiItem*>(file);
            if (!ui_file->save(file_path))
                return false;
        }
        else if (file->itemType() == ProjectItem_scriptFile)
        {
            JZScriptFile *script_file = dynamic_cast<JZScriptFile*>(file);                        
            if (!script_file->save(file_path))
                return false;
        }

        it++;
    }

    for (int i = 0; i < items.size(); i++)    
        onItemChanged(items[i]);
        
    return true;
}

bool JZProject::saveAllItem()
{
    auto items = itemList("./", ProjectItem_any);    
    return saveItems(items);
}

bool JZProject::renameItem(JZProjectItem *item, QString newname)
{    
    QString new_name = item->path() + "/" + newname;
    if (getItem(newname))
    {
        return false;
    }

    item->setName(newname);
    saveItem(item);

    if(isFile(item))
    {
        QString item_path = path() + "/" + item->itemPath();
        QFile f(item_path);
        f.rename(newname);
        save();
    }
    return true;
}

JZProjectItem *JZProject::getItem(QString path)
{    
    if(path.isEmpty() || path == "." || path == "./")
        return &m_root;    
    if (path == "/tmp")
        return &m_tmp;

    JZProjectItem *folder = nullptr;
    if (path.startsWith("/tmp/"))
    {
        folder = &m_tmp;
        path = path.mid(1);
    }
    else
    {
        if (!path.startsWith("./"))
            path = "./" + path;
        folder = &m_root;
    }

    QStringList path_list = path.split("/",Qt::KeepEmptyParts);    
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
        return nullptr;

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
        JZUiItem *ui_file = new JZUiItem();
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

QStringList JZProject::classList()
{
    QStringList ret;

    auto list = itemList("./", ProjectItem_class);
    for (int i = 0; i < list.size(); i++)
    {
        JZScriptClassItem *file = (JZScriptClassItem*)list[i];
        ret << file->className();
    }
    return ret;
}

JZScriptClassItem *JZProject::getItemClass(JZProjectItem *item)
{    
    if (isTmp(item))
        return tmpClassItem(item);

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
        if (isFile(item))
            return item;

        item = item->parent();
    }
    return nullptr;
}

JZScriptFile *JZProject::getScriptFile(JZProjectItem *item)
{
    while (item)
    {
        if (item->itemType() == ProjectItem_scriptFile)
            return (JZScriptFile*)item;

        item = item->parent();
    }
    return nullptr;
}

void JZProject::addGlobalVariable(const QString &name,QString dataType,const QString &value)
{
    globalDefine()->addVariable(name,dataType,value);
}

void JZProject::addGlobalVariable(const QString &name,int dataType,const QString &value)
{
    globalDefine()->addVariable(name,dataType,value);
}

const JZParamDefine *JZProject::globalVariable(QString name)
{   
    if (!mainFile() || !globalDefine())
        return nullptr;

    return globalDefine()->variable(name);
}

QStringList JZProject::globalVariableList()
{    
    if (!mainFile() || !globalDefine())
        return QStringList();

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

const JZFunctionDefine *JZProject::function(QString name)
{
    auto list = itemList("./",ProjectItem_scriptItem);
    for(int i = 0; i < list.size(); i++)
    {
        JZScriptItem *file = (JZScriptItem*)list[i];
        if(file->function().fullName() == name)
            return &file->function();
    }
    return nullptr;
}

JZScriptItem *JZProject::functionItem(QString name)
{
    auto list = itemList("./",ProjectItem_scriptItem);
    for(int i = 0; i < list.size(); i++)
    {
        JZScriptItem *file = (JZScriptItem*)list[i];
        if(file->function().fullName() == name)
            return file;
    }
    return nullptr;
}

QStringList JZProject::functionList()
{
    QStringList ret;

    auto list = itemList("./", ProjectItem_scriptItem);
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
    return indexOfBreakPoint(file,id) >= 0;
}

BreakPoint JZProject::breakPoint(QString file, int id)
{
    int idx = indexOfBreakPoint(file, id);
    if (idx == -1)
        return BreakPoint();

    auto item = getItem(file);
    return m_breakPoints[item][idx];
}

void JZProject::addBreakPoint(const BreakPoint &pt)
{
    if(hasBreakPoint(pt.scriptItemPath,pt.nodeId))
        return;    

    auto item = getItem(pt.scriptItemPath);
    m_breakPoints[item].push_back(pt);
    sigBreakPointChanged(BreakPoint_add, pt.scriptItemPath, pt.nodeId);
}

int JZProject::indexOfBreakPoint(QString file,int id)
{ 
    auto item = getItem(file);
    if(!item || !m_breakPoints.contains(item))
        return -1;

    auto &list = m_breakPoints[item];
    for(int i = 0; i < list.size(); i++)
    {
        if(list[i].nodeId == id)
            return i;
    }
    return -1;
}

void JZProject::removeBreakPoint(QString file,int id)
{    
    if (!hasBreakPoint(file, id))
        return;

    auto item = getItem(file);
    int idx = indexOfBreakPoint(file,id);
    m_breakPoints[item].removeAt(idx);
    if (m_breakPoints[item].size() == 0)
        m_breakPoints.remove(item);

    sigBreakPointChanged(BreakPoint_remove, file, id);
}

QList<BreakPoint> JZProject::breakPoints()
{
    QList<BreakPoint> result;
    auto it = m_breakPoints.begin();
    while (it != m_breakPoints.end())
    {
        auto &list = it.value();
        for(int i = 0; i < it.value().size(); i++)
        {
            auto pt = list[i];
            pt.scriptItemPath = it.key()->itemPath();
            result << pt;
        }
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

void JZProject::onItemChanged(JZProjectItem *item)
{
    if (m_blockRegist)
        return;

    if (isTmp(item))
        return;

    auto registClass = [this](JZScriptClassItem *class_file)
    {
        //起到声明作用
        m_env.objectManager()->delcare(class_file->className());

        //覆盖注册
        auto new_def = class_file->objectDefine();                        
        m_env.objectManager()->replace(new_def);
    };
    
    auto class_file = getItemClass(item);
    if (class_file)
    {
        registClass(class_file);
    }
    else
    {
        if (item->itemType() == ProjectItem_scriptItem)
        {
            auto func_inst = m_env.functionManager();
            JZScriptItem* func = dynamic_cast<JZScriptItem*>(item);
            auto func_def = func->function();
            auto def_ptr = func_inst->function(func_def.fullName());
            if(!def_ptr)
                func_inst->registFunction(func_def);
            else
                func_inst->replaceFunction(func_def);
        }
    }    

    emit sigItemChanged(item);
}

//InitJZProject
JZProjectItem *createScriptFunction() { return new JZScriptItem(JZScriptItem::None); }

void JZProjectInit()
{
    auto inst = JZProjectItemManager::instance();
    inst->registItem(ProjectItem_folder, createJZProjectItem<JZProjectItemFolder>);
    inst->registItem(ProjectItem_ui, createJZProjectItem<JZUiItem>);
    inst->registItem(ProjectItem_param, createJZProjectItem<JZParamItem>);
    inst->registItem(ProjectItem_class, createJZProjectItem<JZScriptClassItem>);
    inst->registItem(ProjectItem_scriptFile, createJZProjectItem<JZScriptFile>);
    inst->registItem(ProjectItem_scriptItem, createScriptFunction);    
}