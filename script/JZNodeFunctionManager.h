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
    void setUserRegist(bool flag);

    QList<const FunctionDefine*> functionList();
    const FunctionDefine *function(QString name);
    void registFunction(const FunctionDefine &define);
    void replaceFunction(const FunctionDefine &define);
    void registCFunction(QString name,bool isFlow,CFunction *func);    
    void unregistFunction(QString name);

    void clearUserReigst();    
    void registCSingle(CSingle *single);
    void unregistCSingle(CSingle *single);

protected:
    JZNodeFunctionManager();
    ~JZNodeFunctionManager();           
    
    QMap<QString, FunctionDefine> m_funcMap;
    QVector<CFunction *> m_cfuncs;
    QVector<CSingle *> m_csingles;
    bool m_userRegist;
    QStringList m_userFuncs;
};

#endif
