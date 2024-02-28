#ifndef JZNODE_FUNCTION_MANAGER_H_
#define JZNODE_FUNCTION_MANAGER_H_

#include <QMap>
#include <QVariant>
#include "JZNode.h"
#include "JZNodeFunctionDefine.h"

typedef bool(*JZNodeFunctionEdit)(JZNode *node);

class JZNodeFunctionManager
{
public:
    static JZNodeFunctionManager *instance();

    void init();        
    void loadLibrary(QString filename);
    void setUserRegist(bool flag);
    void clearUserReigst(); 

    QList<const FunctionDefine*> functionList();
    const FunctionDefine *function(QString name);    
    void registFunction(const FunctionDefine &define);
    void replaceFunction(const FunctionDefine &define);        
    void registCFunction(QString fullName,bool isFlow, QSharedPointer<CFunction> func);
    void unregistFunction(QString name);       

    void registFunctionImpl(JZFunction &impl);
    const JZFunction *functionImpl(QString name);

    JZNodeFunctionEdit editFunction(QString name);
    void registEditFunction(QString name, JZNodeFunctionEdit func);

protected:
    struct Function
    {
        FunctionDefine funcDefine;
        JZFunction funcImpl;
    };

    JZNodeFunctionManager();
    ~JZNodeFunctionManager();           
    
    QMap<QString, Function> m_funcMap;
    QMap<QString, JZNodeFunctionEdit> m_funcEditMap;
    bool m_userRegist;
    QStringList m_userFuncs;
};

#endif
