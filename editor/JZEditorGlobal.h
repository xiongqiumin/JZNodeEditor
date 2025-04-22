#ifndef JZEDITOR_GLOBAL_H_
#define JZEDITOR_GLOBAL_H_

#include "JZScriptEnvironment.h"
#include "JZNodeEditorManager.h"

JZNodeEditorManager *editorManager();
JZScriptEnvironment *editorEnvironment();
JZNodeObjectManager *editorObjectManager();
JZNodeFunctionManager *editorFunctionManager();

#endif