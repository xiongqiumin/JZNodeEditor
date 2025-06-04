#ifndef JZ_MODULE_DEBUG_H_
#define JZ_MODULE_DEBUG_H_

#include "JZNodeCompiler.h"

void JZModuleDebug(JZNodeCompiler *c,int node_id, JZNodeIRParam image,JZNodeIRParam roi);

class JZModuleDebugManager
{
public:
    static JZModuleDebugManager *instance();

    QString nodeDebug() const;
    void setNodeDebug(const QString &funciton);
    

protected:
    JZModuleDebugManager();
    ~JZModuleDebugManager();
    
    QString m_nodeDebug;
};

#endif