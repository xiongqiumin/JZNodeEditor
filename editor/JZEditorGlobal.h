#ifndef JZEDITOR_GLOBAL_H_
#define JZEDITOR_GLOBAL_H_

#include "JZScriptEnvironment.h"

void setEditorEnvironment(JZScriptEnvironment *env);
JZScriptEnvironment *editorEnvironment();
JZNodeObjectManager *editorObjectManager();
JZNodeFunctionManager *editorFunctionManager();

#endif