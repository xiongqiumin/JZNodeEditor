#include "JZUiFile.h"

JZUiFile::JZUiFile()
    :JZProjectItem(ProjectItem_ui)
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
