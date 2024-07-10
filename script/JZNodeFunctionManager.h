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

    QList<const JZFunctionDefine*> functionList();
    const JZFunctionDefine *function(QString name);    
    void registFunction(const JZFunctionDefine &define);
    void replaceFunction(const JZFunctionDefine &define);        
    void registCFunction(const JZFunctionDefine &define, QSharedPointer<CFunction> func);
    void registCFunction(QString fullName,bool isFlow, QSharedPointer<CFunction> func);
    void registBuiltInFunction(const JZFunctionDefine &define, QSharedPointer<BuiltInFunction> func);
    void unregistFunction(QString name);       

    void registFunctionImpl(JZFunction &impl);    
    const JZFunction *functionImpl(QString name);

protected:
    JZNodeFunctionManager();
    ~JZNodeFunctionManager();           
    
    QMap<QString, JZFunctionDefine> m_funcDefine;
    QMap<QString, JZFunction> m_funcImpl;
    bool m_userRegist;
    QStringList m_userFuncs;
};
void updateParam(JZFunctionDefine &dei,CFunction *func);

#endif
