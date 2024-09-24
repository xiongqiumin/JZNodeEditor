#include "JZEditorGlobal.h"
#include "mainwindow.h"

JZScriptEnvironment *editorEnvironment()
{
    return g_mainWindow->project()->environment();
}