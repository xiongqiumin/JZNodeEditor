#include "JZUiFile.h"

JZUiFile::JZUiFile(bool dir)
    :JZProjectItem(ProjectItem_ui,dir)
{
    
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