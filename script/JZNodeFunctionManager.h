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
    void callCFunction(const FunctionDefine *define, const QVariantList &paramIn, QVariantList &paramOut);
    
protected:
    JZNodeFunctionManager();
    ~JZNodeFunctionManager();       
        
    QMap<QString, FunctionDefine> m_funcMap;
};

#endif
