#include "JZModuleDebug.h"

void JZModuleDebug(JZNodeCompiler *c,JZNodeIRParam image,JZNodeIRParam roi)
{
    auto image_debug = JZModuleDebugManager::instance()->imageDebug();
    auto roi_debug = JZModuleDebugManager::instance()->roiDebug();
    if(!image_debug.isEmpty())
        c->addCall(image_debug,{image},{});
    if(!roi_debug.isEmpty())
        c->addCall(roi_debug,{roi},{});
}

JZModuleDebugManager *JZModuleDebugManager::instance()
{
    static JZModuleDebugManager inst;
    return &inst;
}

JZModuleDebugManager::JZModuleDebugManager()
{
}

JZModuleDebugManager::~JZModuleDebugManager()
{
}

QString JZModuleDebugManager::imageDebug() const
{
    return m_imageDebug;
}

QString JZModuleDebugManager::roiDebug() const
{
    return m_roiDebug;
}

void JZModuleDebugManager::setImageDebug(const QString &function)
{
    m_imageDebug = function;
}

void JZModuleDebugManager::setRoiDebug(const QString &function)
{
    m_roiDebug = function;
}