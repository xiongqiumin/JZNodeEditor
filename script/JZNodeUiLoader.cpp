#include "JZNodeUiLoader.h"
#include <QBuffer>
#include <QWidget>
#include "JZNodeBind.h"

JZNodeUiLoader::JZNodeUiLoader()
{

}

JZNodeUiLoader::~JZNodeUiLoader()
{
}

QWidget *JZNodeUiLoader::create(QString xml)
{    
    QBuffer buffer;
    QByteArray data = xml.toUtf8();
    buffer.setData(data);
    return QUiLoader::load(&buffer);
}

QWidget *JZNodeUiLoader::createWidget(const QString &className, QWidget *parent, const QString &name)
{
    if (className == "QWidget" && parent == nullptr)
    {
        auto *w = new jzbind::WidgetWrapper<QWidget>();
        w->setObjectName(name);
        return w;
    }

    return QUiLoader::createWidget(className, parent, name);
}