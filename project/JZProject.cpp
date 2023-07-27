#include <QFile>
#include <QFileInfo>
#include "JZProject.h"
#include "JZNodeFactory.h"
#include "JZScriptFile.h"
#include "JZUiFile.h"
#include "JZEvent.h"
#include "JZNodeFunctionManager.h"

QByteArray ProjectMagic()
{
    QString magic = "1234567";

    QByteArray result;
    QDataStream s(&result,QIODevice::WriteOnly);
    s << QString(magic);

    s << sizeof(JZProject);
    s << sizeof(JZProjectItem);
    s << sizeof(JZScriptFile);
    s << sizeof(JZProjectItemFolder);
    s << sizeof(JZScriptLibraryFile);

    //node
    s << sizeof(JZNodeGemo);
    s << sizeof(JZNodeConnect);
    s << sizeof(JZNode);
    s << sizeof(JZNodeFunction);
    s << sizeof(JZNodeBranch);
    s << sizeof(JZNodeIRCall);
    s << sizeof(JZNodeWhile);
    s << sizeof(JZNodeFunction);

    //node ir
    s << sizeof(JZNodeIR);
    s << sizeof(JZNodeIRNodeId);
    s << sizeof(JZNodeIRCall);
    s << sizeof(JZNodeIRJmp);
    s << sizeof(JZNodeIRExpr);
    s << sizeof(JZNodeIRSet);

    return result;
}


QDataStream &operator<<(QDataStream &s, const JZProject::ItemInfo &param)
{
    s << param.buffer;
    s << param.breakPoints;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZProject::ItemInfo &param)
{
    s >> param.buffer;
    s >> param.breakPoints;
    return s;
}

// JZProject
JZProject::JZProject()    
{        
}

JZProject::~JZProject()
{
}

bool JZProject::isVaild()
{
    return !m_filepath.isEmpty();
}

void JZProject::clear()
{
    m_itemBuffer.clear();
    m_filepath.clear();
    m_root.removeChlids();
    m_root.setName(".");
    m_filepath.clear();
}

void JZProject::init()
{    
    clear();

    JZScriptFile *main_flow = new JZScriptFile(ProjectItem_scriptFlow);
    main_flow->setName("main.jz");
    addItem("",main_flow);

    JZParamFile *param_page = new JZParamFile();
    param_page->setName("param.def");
    addItem("",param_page);

    JZNodeEvent *start = new JZNodeEvent();
    start->setEventType(Event_programStart);
    start->setName("startProgram");
    start->setFlag(Node_propNoRemove);
    main_flow->addNode(JZNodePtr(start));

    saveItem(main_flow);
}

void JZProject::initUi()
{    
    auto func_inst = JZNodeFunctionManager::instance();

    init();        
    addUiClass("./","mainwindow");

    auto main_script = (JZScriptFile *)getItem("./main.jz");
    auto param_def = (JZParamFile *)getItem("./param.def");
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
    func_show->setFunction(func_inst->function("widget.show"));

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

void JZProject::initConsole()
{
    init();
}

void JZProject::registType()
{
    JZNodeFunctionManager::instance()->clearUserReigst();
    JZNodeObjectManager::instance()->clearUserReigst();

    // regist type
    QList<JZProjectItem *> class_list = itemList("./",ProjectItem_class);

    for(int i = 0; i < class_list.size(); i++)
    {
        JZScriptClassFile *class_file = dynamic_cast<JZScriptClassFile*>(class_list[i]);
        auto obj_def = class_file->objectDefine();
        JZNodeObjectManager::instance()->regist(obj_def);
    }
}

bool JZProject::open(QString filepath)
{
    QFile file(filepath);
    if(!file.open(QFile::ReadOnly))
        return false;

    clear();
    m_filepath = filepath;
    QDataStream s(&file);
    loadFromStream(s);
    file.close();

    registType();    
    return true;
}

void JZProject::close()
{
    clear();    
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
    saveToStream(s);
    file.close();    
    return true;
}

void JZProject::saveAllItem()
{
    QList<JZProjectItem*> list = itemList("./",ProjectItem_any);
    for(int i = 0; i < list.size(); i++)
        saveItem(list[i]);
}

void JZProject::saveItem(QString path)
{
    auto item = getItem(path);
    Q_ASSERT(item);
    saveItem(item);
}

void JZProject::saveItem(JZProjectItem *item)
{
    if(item->itemPath() == ".")
        return;

    m_itemBuffer[item->itemPath()].buffer = JZProjectItemFactory::save(item);
}

void JZProject::loadItem(JZProjectItem *item)
{
    QByteArray &buffer = m_itemBuffer[item->itemPath()].buffer;
    QDataStream s(buffer);
    int itemType;
    s >> itemType;
    item->loadFromStream(s);
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

QString JZProject::mainScript()
{
    return "./main.jz";
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
    Q_ASSERT(!item->name().isEmpty());
    item->setProject(this);
    parent->addItem(JZProjectItemPtr(item));
    item->parent()->sort();

    m_itemBuffer[item->itemPath()].buffer = JZProjectItemFactory::save(item);
    return item->parent()->indexOfItem(item);
}

void JZProject::removeItem(QString filepath)
{
    JZProjectItem *item = getItem(filepath);
    auto parent = item->parent();
    int index = parent->indexOfItem(item);
    parent->removeItem(index);

    m_itemBuffer.remove(filepath);
    QString path = filepath + "/";
    auto it = m_itemBuffer.begin();
    while(it != m_itemBuffer.end())
    {
        if(it.key().startsWith(path))
            it = m_itemBuffer.erase(it);
        else
            it++;
    }    
}

int JZProject::renameItem(JZProjectItem *item,QString newname)
{
    QString newPath = item->path() + "/" + newname;
    QString oldPath = item->itemPath();
    item->setName(newname);
    item->parent()->sort();

    m_itemBuffer[newPath] = m_itemBuffer[oldPath];
    m_itemBuffer.remove(oldPath);

    oldPath += "/";

    QList<QPair<QString,QString>> renameList;
    auto it = m_itemBuffer.begin();
    while(it != m_itemBuffer.end())
    {
        if(it.key().startsWith(oldPath))
        {
            QPair<QString,QString> pair;
            pair.first = it.key();

            QString new_key = it.key();
            new_key.replace(oldPath,newPath);
            pair.second = new_key;
            renameList.push_back(pair);
        }
        it++;
    }
    for(int i = 0; i < renameList.size(); i++)
    {
        auto &p = renameList[i];
        m_itemBuffer[p.first] = m_itemBuffer[p.second];
        m_itemBuffer.remove(p.second);
    }    

    return item->parent()->indexOfItem(item);
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

JZProjectItemFolder *JZProject::addFolder(QString path, QString name)
{
    JZProjectItemFolder *folder = new JZProjectItemFolder();
    folder->setName(name);
    addItem(path, folder);
    return folder;
}

JZScriptFile *JZProject::addFunction(QString path, const FunctionDefine &define)
{
    Q_ASSERT(!define.name.isEmpty());

    JZScriptFile *file = new JZScriptFile(ProjectItem_scriptFunction);
    file->setName(define.name);
    addItem(path,file);
    file->setFunction(define);
    return file;
}

void JZProject::removeFunction(QString name)
{
    auto func = getFunction(name);
    if(func)
        removeItem(func->itemPath());
}

JZScriptFile *JZProject::getFunction(QString name)
{
    return nullptr;
}

JZScriptLibraryFile *JZProject::addLibrary(QString name)
{
    return nullptr;
}

void JZProject::removeLibrary(QString name)
{
    removeItem(name);
}

JZScriptLibraryFile *JZProject::getLibrary(QString name)
{
    QStringList list = name.split("::");
    JZProjectItem *item = &m_root;
    for(int i = 0; i < list.size(); i++)
    {
        QList<JZProjectItem*> item_list = item->itemList(ProjectItem_library);
        item = nullptr;
        for(int i = 0; i < item_list.size(); i++)
        {
            if(item_list[i]->name() == list[i])
            {
                item = item_list[i];
                break;
            }
        }
        if(!item)
            return nullptr;
    }
    return dynamic_cast<JZScriptLibraryFile*>(item);
}

JZScriptClassFile *JZProject::addClass(QString path,QString name,QString super)
{    
    QString flow = name + ".jz";

    JZScriptClassFile *class_file = new JZScriptClassFile();
    class_file->init(name,super);

    JZParamFile *data_page = new JZParamFile();
    JZScriptFile *script_flow = new JZScriptFile(ProjectItem_scriptFlow);
    JZProjectItemFolder *script_function = new JZProjectItemFolder();
    class_file->setName(name);
    data_page->setName("变量");
    script_flow->setName("事件");
    script_function->setName("成员函数");

    addItem("./",class_file);
    addItem(class_file->itemPath(),data_page);
    addItem(class_file->itemPath(),script_flow);
    addItem(class_file->itemPath(),script_function);

    return class_file;
}

JZScriptClassFile *JZProject::addUiClass(QString path, QString name)
{
    JZScriptClassFile *file = addClass(path, name,"JZMainWindow");
    if(!file)
        return nullptr;

    JZUiFile *ui_page = new JZUiFile();
    ui_page->setName(name + ".ui");
    addItem(file->itemPath(),ui_page);
    file->reinit();

    return file;
}

JZScriptClassFile *JZProject::getClass(QString name)
{
    QList<JZProjectItem*> list;

    JZProjectItem *item = &m_root;
    int index = name.lastIndexOf("::");
    if(index >= 0)
    {
        QString lib_name = name.left(index);
        item = getLibrary(lib_name);
        name = name.mid(index + 2);
    }
    list = item->itemList(ProjectItem_class);
    for(int i = 0; i < list.size(); i++)
    {
        JZScriptClassFile *file = dynamic_cast<JZScriptClassFile*>(list[i]);
        if(file->name() == name)
            return file;
    }
    return nullptr;
}

JZScriptClassFile *JZProject::getClassFile(JZProjectItem *item)
{    
    while (item)
    {
        if (item->itemType() == ProjectItem_class)
            return (JZScriptClassFile*)item;

        item = item->parent();
    }
    return nullptr;
}

void JZProject::removeClass(QString name)
{
    getClass(name)->uninit();
    removeItem(name);    
}

JZParamDefine *JZProject::globalVariableInfo(QString name)
{   
    QList<JZProjectItem*> list = paramDefineList();
    for(int i = 0; i < list.size(); i++)
    {        
        JZParamFile *file = dynamic_cast<JZParamFile*>(list[i]);
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
        JZParamFile *file = dynamic_cast<JZParamFile*>(list[i]);
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
        JZScriptFile *file = (JZScriptFile*)list[i];
        if(file->function().name == name)
            return &file->function();
    }
    return nullptr;
}

QString JZProject::dir(const QString &filepath)
{
    int index = filepath.lastIndexOf("/");
    return filepath.left(index);
}

bool JZProject::hasBreakPoint(QString file,int id)
{
    auto it = m_itemBuffer.find(file);
    if(it == m_itemBuffer.end())
        return false;

    return it->breakPoints.contains(id);
}

void JZProject::addBreakPoint(QString file,int id)
{
    if(hasBreakPoint(file,id))
        return;

    m_itemBuffer[file].breakPoints.push_back(id);
}

void JZProject::removeBreakPoint(QString file,int id)
{
    auto it = m_itemBuffer.find(file);
    if(it == m_itemBuffer.end())
        return;

    it->breakPoints.removeAll(id);
}

QMap<QString, QVector<int>> JZProject::breakPoints()
{
    QMap<QString, QVector<int>> breakPoints;
    auto it = m_itemBuffer.begin();
    while (it != m_itemBuffer.end())
    {
        breakPoints.insert(it.key(), it->breakPoints);
        it++;
    }
    return breakPoints;
}

void JZProject::saveToStream(QDataStream &s)
{
    s << m_itemBuffer;
}

void JZProject::loadFromStream(QDataStream &s)
{
    QMap<QString,JZProjectItem*> itemMap;

    s >> m_itemBuffer;
    auto it = m_itemBuffer.begin();
    while(it != m_itemBuffer.end())
    {
        itemMap[it.key()] = JZProjectItemFactory::load(it->buffer);
        it++;
    }

    auto it_item = itemMap.begin();
    while(it_item != itemMap.end())
    {
        addItem(dir(it_item.key()),it_item.value());
        it_item++;
    }
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