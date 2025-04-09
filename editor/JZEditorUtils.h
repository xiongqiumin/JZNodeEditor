#ifndef JZ_NODE_EDITOR_UTILS_H_
#define JZ_NODE_EDITOR_UTILS_H_

#include "JZProject.h"

class JZEditorUtils
{
public: 
    static void projectUpdateLayout(JZProject *project);
    static void scriptItemUpdateLayout(JZScriptItem *item);
    static void scriptItemDump(JZScriptItem *item,QString file);
};


#endif
