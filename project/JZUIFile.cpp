#include "JZUiFile.h"
#include "JZNodeObject.h"

JZUiFile::JZUiFile()
    :JZProjectItem(ProjectItem_ui)
{
    m_pri = 9;
}

JZUiFile::~JZUiFile()
{
    
}

void JZUiFile::saveToStream(QDataStream &s)
{
    JZProjectItem::saveToStream(s);
}

void JZUiFile::loadFromStream(QDataStream &s)
{
    JZProjectItem::loadFromStream(s);
}

void JZUiFile::getWidgetMembers(QMap<QString,JZParamDefine> &params)
{
    TestWindow w;
    QList<QWidget*> widgets = w.findChildren<QWidget*>();
    for(int i = 0; i < widgets.size(); i++)
    {
        QString name = widgets[i]->objectName();
        QString class_name = widgets[i]->metaObject()->className();
        int type = JZNodeObjectManager::instance()->getClassIdByCClassName(class_name);
        if(!name.isEmpty() && type != Type_none)
        {
            JZParamDefine def;
            def.name = name;
            def.dataType = type;
            params[name] = def;
        }
    }
}
