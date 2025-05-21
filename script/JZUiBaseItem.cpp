#include "JZUiBaseItem.h"

JZUiBaseItem::JZUiBaseItem()
    :JZProjectItem(ProjectItem_none)
{
}

const JZParamDefine *JZUiBaseItem::widgetVariable(QString name)
{
    for (int i = 0; i < m_widgets.size(); i++)
    {
        if (m_widgets[i].name == name)
            return &m_widgets[i];
    }

    return nullptr;
}

QList<JZParamDefine> JZUiBaseItem::widgets()
{    
    return m_widgets;
}