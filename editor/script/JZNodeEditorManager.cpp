#include <QBuffer>
#include "JZNodeEditorManager.h"
#include "JZNode.h"
#include "JZScriptEnvironment.h"
#include "JZNodeParamDisplayWidget.h"
#include "JZNodeParamEditWidget.h"
#include "JZNodeFlowItem.h"
#include "JZNodeValueItem.h"
#include "JZNodeOperatorItem.h"
#include "JZNodeDisplayItem.h"
#include "JZNodeFunctionItem.h"
#include "modules/camera/JZModuleCameraEditor.h"
#include "modules/communication/JZModuleCommEditor.h"
#include "modules/model/JZModuleModelEditor.h"
#include "modules/opencv/JZModuleOpencvEditor.h"
#include "modules/vision/JZModuleVisionEditor.h"
#include "modules/motion/JZModuleMotionEditor.h"

JZNodeParamDelegate::JZNodeParamDelegate()
{
    editType = Type_none;
    createEdit = nullptr;
    createDisplay = nullptr;
    createParam = nullptr;
    pack = nullptr;
    unpack = nullptr;
}

//image delegate
QVariant createImage(JZScriptEnvironment *env, const QString &value)
{
    QImage *image = new QImage(value);
    return env->objectManager()->objectReferenceVariant(image, true);
}

QByteArray imagePack(JZScriptEnvironment *env,const QVariant &value)
{    
    auto image = env->objectManager()->objectCast<QImage>(value);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image->save(&buffer, "PNG");
    return ba;
}

QVariant imageUnpack(JZScriptEnvironment *env,const QByteArray &buffer)
{    
    QImage *image = new QImage();
    image->loadFromData(buffer);
    return env->objectManager()->objectReferenceVariant(image, true);
}

//JZNodeEditorManager
JZNodeEditorManager *JZNodeEditorManager::instance()
{
    static JZNodeEditorManager inst;
    return &inst;
}

JZNodeEditorManager::JZNodeEditorManager()
{
    m_userRegist = false;    
}

JZNodeEditorManager::~JZNodeEditorManager()
{
    qDeleteAll(m_editorModules);
}

void JZNodeEditorManager::init()
{    
    for(int i = 0; i < m_editorModules.size(); i++)
        m_editorModules[i]->regist(this);
}

void JZNodeEditorManager::registNodeItemCreator(int node_type, CreateJZNodeGraphItemFunc func)
{
    m_nodeItemMap[node_type] = func;
}

bool JZNodeEditorManager::hasNodeItemCreator(int node_type)
{
    return m_nodeItemMap.contains(node_type);
}

JZNodeGraphItem* JZNodeEditorManager::createNodeItem(JZNode* node)
{
    int node_type = node->type();
    if (m_nodeItemMap.contains(node_type))
        return m_nodeItemMap[node_type](node);
    else
        return new JZNodeGraphItem(node);
}

void JZNodeEditorManager::setUserRegist(bool flag)
{
    m_userRegist = true;
}

void JZNodeEditorManager::clearUserRegist()
{
    m_userRegist = false;    
    for (auto d : m_userDelegateList)
        m_delegateMap.remove(d);
    
    m_userDelegateList.clear();
}

void JZNodeEditorManager::addModule(JZModuleEditor *module)
{
    m_editorModules.push_back(module);
}

void JZNodeEditorManager::registLogicNode(int node_type,QString path, QString icon, CreateJZNodeGraphItemFunc func)
{
    JZLogicNode logic;
    logic.nodeType = node_type;
    logic.path = path;
    logic.icon = icon;
    m_logicNode.push_back(logic);

    if(func)
        registNodeItemCreator(node_type, func);
}

void JZNodeEditorManager::registLogicNode(JZLogicNode logic)
{
    m_logicNode.push_back(logic);
}

QList<JZLogicNode>  JZNodeEditorManager::logicNodeList()
{
    return m_logicNode;
}

void JZNodeEditorManager::registDelegate(int data_type, JZNodeParamDelegate delegate)
{
    m_delegateMap[data_type] = delegate;
    if (m_userRegist)
        m_userDelegateList << data_type;
}

JZNodeParamDelegate *JZNodeEditorManager::delegate(int data_type)
{
    if (!m_delegateMap.contains(data_type))
        return nullptr;

    return &m_delegateMap[data_type];
}


//JZNodeEditorInit
void JZNodeEditorInit()
{
    auto inst = JZNodeEditorManager::instance();

    inst->registNodeItemCreator(Node_for, CreateJZNodeGraphItem<JZNodeForItem>);
    inst->registNodeItemCreator(Node_foreach, CreateJZNodeGraphItem<JZNodeForeachItem>);
    inst->registNodeItemCreator(Node_if, CreateJZNodeGraphItem<JZNodeIfItem>);
    inst->registNodeItemCreator(Node_switch, CreateJZNodeGraphItem<JZNodeSwitchItem>);
    inst->registNodeItemCreator(Node_tryCatch, CreateJZNodeGraphItem<JZNodeTryCatchItem>);

    for (int i = Node_add; i < Node_expr; i++)
    {
        inst->registNodeItemCreator(i, CreateJZNodeGraphItem<JZNodeOperatorItem>);
    }
    inst->registNodeItemCreator(Node_expr, CreateJZNodeGraphItem<JZNodeExpressionItem>);

    inst->registNodeItemCreator(Node_function, CreateJZNodeGraphItem<JZNodeFunctionItem>);

    inst->registNodeItemCreator(Node_literal, CreateJZNodeGraphItem<JZNodeLiteralItem>);    
    inst->registNodeItemCreator(Node_setParam, CreateJZNodeGraphItem<JZNodeSetParamItem>);
    inst->registNodeItemCreator(Node_param, CreateJZNodeGraphItem<JZNodeParamItem>);    
    inst->registNodeItemCreator(Node_enum, CreateJZNodeGraphItem<JZNodeEnumItem>);
    inst->registNodeItemCreator(Node_flag, CreateJZNodeGraphItem<JZNodeFlagItem>);

    inst->registNodeItemCreator(Node_display, CreateJZNodeGraphItem<JZNodeDisplayItem>);

    JZCameraEditorInit();
    JZModuleCommEditorInit();
    JZModuleModelEditorInit();
    JZModuleOpencvEditorInit();
    JZModuleVisionEditorInit();
    JZModuleMotionEditorInit();
}
