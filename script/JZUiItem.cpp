#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDomDocument>
#include "JZUiItem.h"
#include "JZNodeObject.h"
#include "JZProject.h"

JZUiItem::JZUiItem()
    :JZProjectItem(ProjectItem_ui)
{
    m_name = "ui";
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

JZUiItem::~JZUiItem()
{
    
}

bool JZUiItem::save(QString filepath)
{
    QFile file(filepath);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return false;

    QTextStream s(&file);
    s.setCodec("utf-8");
    s << m_xml;
    
    return true;
}

bool JZUiItem::load(QString filepath)
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

void JZUiItem::saveToStream(QDataStream &s) const
{
    s << m_xml;
}

bool JZUiItem::loadFromStream(QDataStream &s)
{
    QString xml;
    s >> xml;
    setXml(xml);
    return true;
}

QString JZUiItem::xml()
{
    return m_xml;
}

void JZUiItem::setXml(QString xml)
{
    m_xml = xml;
    updateDefine();    
}

const JZParamDefine *JZUiItem::widgetVariable(QString name)
{
    for (int i = 0; i < m_widgets.size(); i++)
    {
        if (m_widgets[i].name == name)
            return &m_widgets[i];
    }

    return nullptr;
}

QList<JZParamDefine> JZUiItem::widgets()
{    
    return m_widgets;
}

void JZUiItem::walkChild(const QDomElement &root)
{
    auto ele_list = root.childNodes();
    for (int ele_idx = 0; ele_idx < ele_list.size(); ele_idx++)
    {
        auto sub_ele = ele_list.at(ele_idx).toElement();
        if (sub_ele.nodeName() == "widget")
        {
            QString class_name = sub_ele.attribute("class");
            QString obj_name = sub_ele.attribute("name");

            auto meta = project()->environment()->objectManager()->meta(class_name);
            if (meta)
            {
                JZParamDefine def;
                def.name = obj_name;
                def.type = class_name;
                m_widgets.push_back(def);
            }
        }
        walkChild(sub_ele);
    }
}

void JZUiItem::updateDefine()
{
    QList<JZParamDefine> list;

    QDomDocument doc;
    bool ret = doc.setContent(m_xml);
    Q_ASSERT(ret);

    m_widgets.clear();

    auto root = doc.documentElement();
    auto ele_list = root.childNodes();
    for (int ele_idx = 0; ele_idx < ele_list.size(); ele_idx++)
    {
        auto sub_ele = ele_list.at(ele_idx).toElement();
        walkChild(sub_ele);
    }
    
    std::sort(m_widgets.begin(), m_widgets.end(), [](const JZParamDefine &def1,const JZParamDefine &def2)->bool {
        return def1.name < def2.name;
    });
}