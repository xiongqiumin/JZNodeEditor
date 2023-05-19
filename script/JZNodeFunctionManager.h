#ifndef JZNODE_FUNCTION_MANAGER_H_
#define JZNODE_FUNCTION_MANAGER_H_

#include <QMap>
#include <QVariant>
#include "JZNode.h"
#include "JZNodeFunctionDefine.h"

class JZNodeFunctionManager
{
public:
    static JZNodeFunctionManager *instance();

    void init();        
    void loadLibrary(QString filename);

    QList<const FunctionDefine*> functionList();
    const FunctionDefine *function(QString name);
    void registFunction(const FunctionDefine &define);
    void registCFunction(QString name,CFunction *func);
    
protected:
    JZNodeFunctionManager();
    ~JZNodeFunctionManager();       
    int idToType(QString id);
    
    QMap<QString, FunctionDefine> m_funcMap;
    QVector<CFunction *> m_cfuncs;
};

#endif
