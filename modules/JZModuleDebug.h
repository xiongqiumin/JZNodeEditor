#ifndef JZ_MODULE_DEBUG_H_
#define JZ_MODULE_DEBUG_H_

#include "JZNodeCompiler.h"

void JZModuleDebug(JZNodeCompiler *c,int node_id, JZNodeIRParam image,JZNodeIRParam roi);

class JZModuleDebugManager
{
public:
    static JZModuleDebugManager *instance();

    QString imageDebug() const;
    QString roiDebug() const;
    void setImageDebug(const QString &funciton);
    void setRoiDebug(const QString &funciton);

protected:
    JZModuleDebugManager();
    ~JZModuleDebugManager();
    
    QString m_imageDebug;
    QString m_roiDebug;
};

#endif