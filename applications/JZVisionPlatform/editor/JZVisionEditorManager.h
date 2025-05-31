#ifndef JZNODE_EDITOR_MANAGER_H_
#define JZNODE_EDITOR_MANAGER_H_

#include <QString>
#include <QMap>
#include "JZNode.h"

class JZVisionEditorManager
{
public:
    static JZVisionEditorManager *instance();

    JZVisionEditorManager();
    ~JZVisionEditorManager();    
    
};

void JZVisionEditorInit();

#endif // !JZNODE_EDITOR_MANAGER_H_
