#ifndef JZEDITOR_GLOBAL_H_
#define JZEDITOR_GLOBAL_H_

#include <QMenuBar>
#include <QToolBar>
#include "JZScriptEnvironment.h"
#include "JZNodeEditorManager.h"
#include "JZProject.h"

JZProject* editorProject();
void setEditorProject(JZProject* project);

QMenuBar* editorMenuBar();
void setEditorMenuBar(QMenuBar* project);

QToolBar* editorToolBar();
void setEditorToolBar(QToolBar* project);

JZNodeEditorManager *editorManager();
JZScriptEnvironment *editorEnvironment();
JZNodeObjectManager *editorObjectManager();
JZNodeFunctionManager *editorFunctionManager();
JZNodeFactory *editorNodeFactory();

#endif