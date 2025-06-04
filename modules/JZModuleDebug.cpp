#include "JZModuleDebug.h"

void JZModuleDebug(JZNodeCompiler *c,int node_id, JZNodeIRParam image,JZNodeIRParam roi)
{    
    auto node_debug = JZModuleDebugManager::instance()->nodeDebug();
    if (!node_debug.isEmpty())
    {
        QList<JZNodeIRParam> in_list;
        in_list << irLiteral(node_id) << image;
        if (!roi.isNull())
            in_list << roi;
        c->addCall(node_debug, in_list, {});
    }
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

QString JZModuleDebugManager::nodeDebug() const
{
    return m_nodeDebug;
}

void JZModuleDebugManager::setNodeDebug(const QString &function)
{
    m_nodeDebug = function;
}