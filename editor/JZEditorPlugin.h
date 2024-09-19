#ifndef JZ_EDITOR_PLUGIN_H_
#define JZ_EDITOR_PLUGIN_H_

#include <QtCore/QtPlugin>
#include "JZModule.h"

#define JZEditorPluginInterface_iid "Plugin.JZEditorPluginInterface"

class JZEditorPluginInterface
{
public:
    JZEditorPluginInterface();
    virtual ~JZEditorPluginInterface();

    /// @brief 获取插件名
    virtual JZModuleInfo module() = 0;    
};

Q_DECLARE_INTERFACE(JZEditorPluginInterface, JZEditorPluginInterface_iid);





















#endif
