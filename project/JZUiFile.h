#ifndef JZNODE_UI_FILE_H_
#define JZNODE_UI_FILE_H_

#include "JZProjectItem.h"

//JZUiFile
class JZUiFile : public JZProjectItem
{
public:
    JZUiFile(bool dir);
    virtual ~JZUiFile();

    virtual void saveToStream(QDataStream &s);
    virtual void loadFromStream(QDataStream &s);
};



#endif
