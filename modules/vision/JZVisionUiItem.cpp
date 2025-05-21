#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDomDocument>
#include "JZUiExtItem.h"
#include "JZNodeObject.h"
#include "JZProject.h"

JZUiExtItem::JZUiExtItem()
    :JZProjectItem(ProjectItem_ui)
{
    m_name = "ui";
}

JZUiExtItem::~JZUiExtItem()
{
    
}

void JZUiExtItem::saveToStream(QDataStream &s) const
{
}

bool JZUiExtItem::loadFromStream(QDataStream &s)
{
}