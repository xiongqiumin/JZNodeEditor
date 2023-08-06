#include "JZUiFile.h"
#include "JZNodeObject.h"
#include "JZNodeUiLoader.h"
#include <QDebug>

JZUiFile::JZUiFile()
    :JZProjectItem(ProjectItem_ui)
{
    m_pri = 9;
}

JZUiFile::~JZUiFile()
{
    
}

QString JZUiFile::xml()
{
    return m_xml;
}

void JZUiFile::setXml(QString xml)
{
    m_xml = xml;
}

void JZUiFile::saveToStream(QDataStream &s)
{
    JZProjectItem::saveToStream(s);
    s << m_xml;
}

void JZUiFile::loadFromStream(QDataStream &s)
{
    JZProjectItem::loadFromStream(s);
    s >> m_xml;
}

void JZUiFile::updateDefine(JZNodeObjectDefine &def)
{        
    def.isUiWidget = true;
    def.widgteXml = m_xml;
       
    JZNodeUiLoader loader;
    QWidget *root = loader.create(m_xml);
    Q_ASSERT(root);
    QList<QWidget*> widgets = root->findChildren<QWidget*>();
    QMap<QString, JZParamDefine> &params = def.params;
    for(int i = 0; i < widgets.size(); i++)
    {
        QString name = widgets[i]->objectName();
        QString class_name = widgets[i]->metaObject()->className();
        int type = JZNodeObjectManager::instance()->getClassIdByQObject(class_name);
        if(!name.isEmpty() && type != Type_none)
        {
            JZParamDefine def;
            def.name = name;
            def.dataType = type;
            def.cref = true;
            params[name] = def;
        }
    }
}
