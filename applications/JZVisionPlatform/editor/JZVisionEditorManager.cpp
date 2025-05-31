#include <QBuffer>
#include "JZVisionEditorManager.h"

JZVisionEditorManager *JZVisionEditorManager::instance()
{
    static JZVisionEditorManager inst;
    return &inst;
}

JZVisionEditorManager::JZVisionEditorManager()
{
}

JZVisionEditorManager::~JZVisionEditorManager()
{
}

//JZVisionEditorInit
void JZVisionEditorInit()
{
    auto inst = JZVisionEditorManager::instance();
}
