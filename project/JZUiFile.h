#ifndef JZNODE_UI_FILE_H_
#define JZNODE_UI_FILE_H_

#include "JZProjectItem.h"
#include "JZNode.h"

//JZUiFile
class JZUiFile : public JZProjectItem
{
public:
    JZUiFile();
    virtual ~JZUiFile();

    virtual void saveToStream(QDataStream &s);
    virtual void loadFromStream(QDataStream &s);

    void getWidgetMembers(QMap<QString,JZParamDefine> &params);
};



#endif
