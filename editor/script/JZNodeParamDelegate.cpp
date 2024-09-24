#include <QBuffer>
#include "JZNodeParamDelegate.h"
#include "JZTx.h"

//JZNodeParamDelegate
JZNodeParamDelegate::JZNodeParamDelegate()
{
    editType = Type_none;
    createEdit = nullptr;
    createParam = nullptr;
    createDisplay = nullptr;
    pack = nullptr;
    unpack = nullptr;
}

//JZNodeParamDelegateManager
JZNodeParamDelegateManager *JZNodeParamDelegateManager::instance()
{
    static JZNodeParamDelegateManager inst;
    return &inst;
}

JZNodeParamDelegateManager::JZNodeParamDelegateManager()
{
}

JZNodeParamDelegateManager::~JZNodeParamDelegateManager()
{
}

void JZNodeParamDelegateManager::registDelegate(int data_type,JZNodeParamDelegate delegate)
{
    m_delegateMap[data_type] = delegate;
}

bool JZNodeParamDelegateManager::hasDelegate(int data_type)
{
    return m_delegateMap.contains(data_type);
}

JZNodeParamDelegate *JZNodeParamDelegateManager::delegate(int data_type)
{
    if(!m_delegateMap.contains(data_type))
        return nullptr;

    return &m_delegateMap[data_type];
}

CreateParamEditFunc JZNodeParamDelegateManager::editFunction(int data_type)
{
    if (!m_delegateMap.contains(data_type))
        return nullptr;

    return m_delegateMap[data_type].createEdit;
}

CreateParamDisplayFunc JZNodeParamDelegateManager::displayFunction(int data_type)
{
    if (!m_delegateMap.contains(data_type))
        return nullptr;

    return m_delegateMap[data_type].createDisplay;
}

//init delegate
QVariant createImage(const QString &value)
{    
    QImage *image = new QImage(value);
    return editorObjectManager()->objectRefrence(image, true);
}

QByteArray imagePack(const QVariant &value)
{
    JZTX_FUNCTION
    
    auto image = editorObjectManager()->objectCast<QImage>(value);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image->save(&buffer, "PNG");    
    return ba;
}

QVariant imageUnpack(const QByteArray &buffer)
{
    JZTX_FUNCTION

    QImage *image = new QImage();    
    image->loadFromData(buffer);
    return editorObjectManager()->objectRefrence(image,true);
}


void InitParamDelegate()
{
    auto inst = JZNodeParamDelegateManager::instance();

    JZNodeParamDelegate d_image;
    d_image.editType = Type_imageEdit;
    d_image.createDisplay = CreateParamDisplayWidget<JZNodeImageDisplayWidget>;
    d_image.createParam = createImage;    
    d_image.pack = imagePack;
    d_image.unpack = imageUnpack;
    inst->registDelegate(Type_image, d_image);

    JZNodeParamDelegate d_imageEdit;
    d_imageEdit.createEdit = CreateParamEditWidget<JZNodeImageEditWidget>;
    inst->registDelegate(Type_imageEdit,d_imageEdit);
}