#ifndef JZEDITOR_GLOBAL_H_
#define JZEDITOR_GLOBAL_H_

#include "JZScriptEnvironment.h"

JZScriptEnvironment *editorEnvironment();
JZNodeObjectManager *editorObjectManager();
JZNodeFunctionManager *editorFunctionManager();

#endif