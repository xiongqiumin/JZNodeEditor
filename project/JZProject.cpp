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

// JZProject
JZProject::JZProject()    
{        
}

JZProject::~JZProject()
{
}

void JZProject::clear()
{
    m_itemBuffer.clear();
    m_filepath.clear();
    m_root.removeChlids();
    m_root.setName(".");
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
    addUiClass("mainwindow");    

    auto main_script = (JZScriptFile *)getItem("./main.jz");
    auto param_def = (JZParamFile *)getItem("./param.def");
    param_def->addVariable("mainwindow",JZClassId("mainwindow"));

    JZNodeSetParam *set_param = new JZNodeSetParam();    
    JZNodeCreate *create = new JZNodeCreate();    
    JZNodeFunction *func_show = new JZNodeFunction();
    main_script->addNode(JZNodePtr(create));
    main_script->addNode(JZNodePtr(set_param));
    main_script->addNode(JZNodePtr(func_show));

    set_param->setVariable("mainwindow");
    create->setClassName("mainwindow");
    func_show->setFunction(func_inst->function("widget.show"));

    JZNode *start = main_script->getNode(0);   
    main_script->addConnect(start->flowOutGemo(),create->flowInGemo());

    main_script->addConnect(create->flowOutGemo(),set_param->flowInGemo());
    main_script->addConnect(create->paramOutGemo(0),set_param->paramInGemo(0));

    main_script->addConnect(set_param->flowOutGemo(0),func_show->flowInGemo());
    main_script->addConnect(set_param->paramOutGemo(0),func_show->paramInGemo(0));

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
    QDataStream s(&file);
    loadFromStream(s);
    file.close();

    registType();
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

void JZProject::saveItem(JZProjectItem *item)
{
    if(item->itemPath() == ".")
        return;

    m_itemBuffer[item->itemPath()] = JZProjectItemFactory::save(item);
}

void JZProject::loadItem(JZProjectItem *item)
{
    QByteArray &buffer = m_itemBuffer[item->itemPath()];
    QDataStream s(buffer);
    int itemType;
    s >> itemType;
    item->loadFromStream(s);
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

    m_itemBuffer[item->itemPath()] = JZProjectItemFactory::save(item);
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

JZScriptFile *JZProject::addFunction(const FunctionDefine &define)
{
    Q_ASSERT(!define.name.isEmpty());

    JZScriptFile *file = new JZScriptFile(ProjectItem_scriptFunction);
    file->setName(define.name);
    addItem("./",file);
    file->setFunction(define);
    return file;
}

void JZProject::removeFunction(QString name)
{
    removeItem(name);
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

JZScriptClassFile *JZProject::addClass(QString name,QString super)
{    
    QString flow = name + ".jz";

    JZScriptClassFile *class_file = new JZScriptClassFile();
    class_file->init(name,super);

    JZParamFile *data_page = new JZParamFile();
    JZScriptFile *script_flow = new JZScriptFile(ProjectItem_scriptFlow);
    JZScriptLibraryFile *script_function = new JZScriptLibraryFile();
    class_file->setName(name);
    data_page->setName("变量");
    script_flow->setName("事件");
    script_flow->setBindClass(name);
    script_function->setName("函数");

    addItem("./",class_file);
    addItem(class_file->itemPath(),data_page);
    addItem(class_file->itemPath(),script_flow);
    addItem(class_file->itemPath(),script_function);

    return class_file;
}

JZScriptClassFile *JZProject::addUiClass(QString name)
{
    JZScriptClassFile *file = addClass(name,"JZMainWindow");
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

void JZProject::removeClass(QString name)
{
    getClass(name)->unint();
    removeItem(name);    
}

JZParamDefine *JZProject::getVariableInfo(QString name)
{
    int index = name.indexOf(".");
    QString base_name = name;
    QString member_name;
    if(index != -1)
    {
        base_name = name.left(index);
        member_name = name.mid(index + 1);
    }

    QList<JZProjectItem*> list = itemList("./",ProjectItem_param);
    for(int i = 0; i < list.size(); i++)
    {
        JZParamFile *file = dynamic_cast<JZParamFile*>(list[i]);
        JZParamDefine *def = file->getVariable(base_name);
        if(def)
        {
            if(member_name.isEmpty())
                return def;
            else
            {
                auto meta = JZNodeObjectManager::instance()->meta(def->dataType);
                return meta->param(member_name);
            }
        }
    }
    return nullptr;
}

void JZProject::setVariable(QString name,const QVariant &value)
{
    JZParamDefine *def = getVariableInfo(name);
    Q_ASSERT(def);
    def->value = value;
}

QVariant JZProject::getVariable(QString name)
{
    JZParamDefine *def = getVariableInfo(name);
    Q_ASSERT(def);
    return def->value;
}

QStringList JZProject::variableList()
{    
    QStringList result;
    QList<JZProjectItem*> list = itemList("./",ProjectItem_param);
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
    auto it = m_breakPoints.find(file);
    if(it == m_breakPoints.end())
        return false;

    return it->contains(id);
}

void JZProject::addBreakPoint(QString file,int id)
{
    if(hasBreakPoint(file,id))
        return;

    m_breakPoints[file].push_back(id);
}

void JZProject::removeBreakPoint(QString file,int id)
{
    auto it = m_breakPoints.find(file);
    if(it == m_breakPoints.end())
        return;

    it->removeAll(id);
    if(it->size() == 0)
        m_breakPoints.erase(it);
}

QVector<int> JZProject::breakPoints(QString file)
{
    return m_breakPoints.value(file,QVector<int>());
}

void JZProject::saveToStream(QDataStream &s)
{
    s << m_itemBuffer;
    s << m_breakPoints;
}

void JZProject::loadFromStream(QDataStream &s)
{
    QMap<QString,JZProjectItem*> itemMap;

    s >> m_itemBuffer;
    auto it = m_itemBuffer.begin();
    while(it != m_itemBuffer.end())
    {
        itemMap[it.key()] = JZProjectItemFactory::load(it.value());
        it++;
    }

    auto it_item = itemMap.begin();
    while(it_item != itemMap.end())
    {
        addItem(dir(it_item.key()),it_item.value());
        it_item++;
    }
    s >> m_breakPoints;
}
