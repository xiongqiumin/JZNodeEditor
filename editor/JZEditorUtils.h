#ifndef JZ_EDITOR_UTILS_H_
#define JZ_EDITOR_UTILS_H_

#include "JZProject.h"

class JZNodeUtils
{
public: 
    static void projectUpdateLayout(JZProject *project);
    static void scriptItemUpdateLayout(JZScriptItem *item);
    static void scriptItemDump(JZScriptItem *item,QString file);
};


#elif
