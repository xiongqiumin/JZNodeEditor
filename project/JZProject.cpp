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

void JZProject::init()
{
    m_root.removeChlids();
    m_filepath.clear();    

    m_root.setName(".");

    JZScriptFile *flow_page = new JZScriptFile(ProjectItem_scriptFlow);    
    flow_page->setName("main.jz");    
    addItem("",flow_page);

    JZScriptParamDefineFile *param_page = new JZScriptParamDefineFile();
    param_page->setName("param.def");
    addItem("",param_page);

    JZNodeEvent *start = new JZNodeEvent();
    start->setEventType(Event_programStart);
    start->setName("startProgram");
    flow_page->addNode(JZNodePtr(start));
}

void JZProject::initUi()
{    
    auto func_inst = JZNodeFunctionManager::instance();

    init();        
    addUiClass("mainwindow");    

    auto main_script = (JZScriptFile *)getItem("./main.jz");
    auto param_def = (JZScriptParamDefineFile *)getItem("./param.def");
    param_def->addVariable("mainwindow",JZClassId("mainwindow"));

    JZNodeSetParam *set_param = new JZNodeSetParam();
    set_param->setParamId("mainwindow",true);

    JZNodeCreate *create = new JZNodeCreate();
    create->setClassName("mainwindow");

    JZNodeFunction *func_show = new JZNodeFunction();
    func_show->setFunction(func_inst->function("widget.show"));

    JZNode *start = main_script->getNode(0);

    main_script->addNode(JZNodePtr(create));
    main_script->addNode(JZNodePtr(set_param));
    main_script->addNode(JZNodePtr(func_show));

    main_script->addConnect(start->flowOutGemo(),create->flowInGemo());

    main_script->addConnect(create->flowOutGemo(),set_param->flowInGemo());
    main_script->addConnect(create->paramOutGemo(0),set_param->paramInGemo(0));

    main_script->addConnect(set_param->flowOutGemo(0),func_show->flowInGemo());
    main_script->addConnect(set_param->paramOutGemo(0),func_show->paramInGemo(0));
}

void JZProject::initConsole()
{
    init();
}

bool JZProject::open(QString filepath)
{
    QFile file(filepath);
    if(!file.open(QFile::ReadOnly))
        return false;

    QDataStream s(&file);
    loadFromStream(s);

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
    saveToStream(s);
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
    if(!path.startsWith("./"))
        path += "./";
    if(path == "." || path == "./")
        return &m_root;

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
    m_root.addItem(JZProjectItemPtr(file));
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

    JZScriptParamDefineFile *data_page = new JZScriptParamDefineFile();
    JZScriptFile *script_flow = new JZScriptFile(ProjectItem_scriptFlow);
    JZScriptLibraryFile *script_function = new JZScriptLibraryFile();
    class_file->setName(name);
    data_page->setName("变量");
    script_flow->setName("事件");
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
    QList<JZProjectItem*> list = itemList("./",ProjectItem_param);
    for(int i = 0; i < list.size(); i++)
    {
        JZScriptParamDefineFile *file = dynamic_cast<JZScriptParamDefineFile*>(list[i]);
        JZParamDefine *def = file->getVariable(name);
        if(def)
            return def;
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
        JZScriptParamDefineFile *file = dynamic_cast<JZScriptParamDefineFile*>(list[i]);
        result << file->variableList();
    }
    return result;
}

QList<JZProjectItem *> JZProject::itemList(QString path,int type)
{    
    return getItem(path)->itemList(type);    
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
    m_root.saveToStream(s);
}

void JZProject::loadFromStream(QDataStream &s)
{    
    m_root.loadFromStream(s);
}
