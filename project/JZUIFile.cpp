#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "JZUiFile.h"
#include "JZNodeObject.h"
#include "JZNodeUiLoader.h"

JZUiFile::JZUiFile()
    :JZProjectItem(ProjectItem_ui)
{
    m_pri = 9;
    m_xml =  R"(<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>widget</class>
 <widget class="QWidget" name="widget">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>640</width>
    <height>480</height>
   </rect>
  </property>        
 </widget>
 <resources/>
 <connections/>
</ui>
)";

}

JZUiFile::~JZUiFile()
{
    
}

bool JZUiFile::save(QString filepath)
{
    QFile file(filepath);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return false;

    QTextStream s(&file);
    s.setCodec("utf-8");
    s << m_xml;
    return true;
}

bool JZUiFile::load(QString filepath)
{
    QFile file(filepath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    m_name = QFileInfo(filepath).fileName();
    QTextStream s(&file);
    s.setCodec("utf-8");
    setXml(s.readAll());
    return true;
}

QString JZUiFile::xml()
{
    return m_xml;
}

void JZUiFile::setXml(QString xml)
{
    m_xml = xml;
    updateDefine();
    regist();
}

QList<JZParamDefine> JZUiFile::widgets()
{    
    return m_widgets;
}

void JZUiFile::updateDefine()
{
    QList<JZParamDefine> list;

    JZNodeUiLoader loader;
    QWidget *root = loader.create(m_xml);
    Q_ASSERT(root);

    QList<QWidget*> widgets = root->findChildren<QWidget*>();    
    for (int i = 0; i < widgets.size(); i++)
    {
        QString name = widgets[i]->objectName();
        QString class_name = widgets[i]->metaObject()->className();

        int type = JZNodeObjectManager::instance()->getClassIdByQObject(class_name);
        if (!name.isEmpty() && type != Type_none)
        {
            JZParamDefine def;
            def.name = name;
            def.type = JZNodeType::typeToName(type);
            list << def;
        }
    }
    m_widgets = list;
    delete root;
}