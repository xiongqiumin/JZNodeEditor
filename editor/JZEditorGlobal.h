#ifndef JZEDITOR_GLOBAL_H_
#define JZEDITOR_GLOBAL_H_

#include "JZScriptEnvironment.h"

JZCORE_EXPORT void setEditorEnvironment(JZScriptEnvironment *env);
JZScriptEnvironment *editorEnvironment();
JZNodeObjectManager *editorObjectManager();
JZNodeFunctionManager *editorFunctionManager();

#endif