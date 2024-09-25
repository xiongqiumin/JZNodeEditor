#include <QBuffer>
#include "JZNodeEditorManager.h"
#include "JZNode.h"
#include "JZScriptEnvironment.h"
#include "JZNodeParamDisplayWidget.h"
#include "JZNodeParamEditWidget.h"
#include "JZTx.h"

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
    return env->objectManager()->objectRefrence(image, true);
}

QByteArray imagePack(JZScriptEnvironment *env,const QVariant &value)
{
    JZTX_FUNCTION

    auto image = env->objectManager()->objectCast<QImage>(value);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image->save(&buffer, "PNG");
    return ba;
}

QVariant imageUnpack(JZScriptEnvironment *env,const QByteArray &buffer)
{
    JZTX_FUNCTION

    QImage *image = new QImage();
    image->loadFromData(buffer);
    return env->objectManager()->objectRefrence(image, true);
}

//JZNodeEditorManager
JZNodeEditorManager::JZNodeEditorManager(JZScriptEnvironment *env)
{
    m_userRegist = false;
    m_env = env;
}

JZNodeEditorManager::~JZNodeEditorManager()
{
}

JZScriptEnvironment *JZNodeEditorManager::env()
{
    return m_env;
}

void JZNodeEditorManager::init()
{    
    JZNodeParamDelegate d_image;
    d_image.editType = Type_imageEdit;
    d_image.createDisplay = CreateParamDisplayWidget<JZNodeImageDisplayWidget>;
    d_image.createParam = createImage;
    d_image.pack = imagePack;
    d_image.unpack = imageUnpack;
    registDelegate(Type_image, d_image);

    JZNodeParamDelegate d_imageEdit;
    d_imageEdit.createEdit = CreateParamEditWidget<JZNodeImageEditWidget>;
    registDelegate(Type_imageEdit, d_imageEdit);
}

void JZNodeEditorManager::setUserRegist(bool flag)
{
    m_userRegist = true;
}

void JZNodeEditorManager::clearUserRegist()
{
    m_userRegist = false;
    for (auto f : m_userFunctionList)
        m_functionMap.remove(f);
    for (auto d : m_userDelegateList)
        m_delegateMap.remove(d);

    m_userFunctionList.clear();
    m_userDelegateList.clear();
}

void JZNodeEditorManager::registCustomFunctionNode(QString function, int node_type)
{
    Q_ASSERT(!m_functionMap.contains(function));
    m_functionMap[function] = node_type;
    if (m_userRegist)
        m_userFunctionList << function;
}

int JZNodeEditorManager::customFunctionNode(QString function)
{
    return m_functionMap.value(function, Node_none);
}

void JZNodeEditorManager::unregistCustomFunctionNode(QString function)
{
    m_functionMap.remove(function);
    m_userFunctionList.removeAll(function);
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