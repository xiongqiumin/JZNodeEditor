#ifndef AS_TOJZSCRIPT_H_
#define AS_TOJZSCRIPT_H_

#include <QString>
#include "JZProject.h"

class asCScriptNode;
class JZCORE_EXPORT ASConvert
{
public:
    ASConvert();
    ~ASConvert();

    bool convert(QString script,JZScriptFile *file);
    QString error();

protected:
    template<class T>
    T *createNode(){
        T *node = new T();
        m_script->addNode(node);
        return node;
    }

    void nodeDebug(asCScriptNode *root,QString &result,int level);
    QString nodeDebug(asCScriptNode *node);
    void printNode(asCScriptNode *node);
    QString nodeText(asCScriptNode *node);
    asCScriptNode *nextNode(asCScriptNode *node,int count);
    QList<asCScriptNode*> childList(asCScriptNode *node);

    bool addFunction(asCScriptNode *node);
    bool addClass(asCScriptNode *node);
    bool addDeclaration(asCScriptNode *node);

    QList<JZParamDefine> toParamList(asCScriptNode *node);
    JZNode *toReturn(asCScriptNode *node);
    JZNode *toIf(asCScriptNode *node);
    void setNodeIf(JZNodeIf *node_if,asCScriptNode *as_node,int cond);
	JZNode *toFor(asCScriptNode *node);
    JZNode *toStatement(asCScriptNode *node);
    QList<JZNode*> toStatementBlock(asCScriptNode *node);
    JZNode *toExpression(asCScriptNode *node);
    JZNode *toExprTerm(asCScriptNode *node);
    JZNode *toAssignment(asCScriptNode *node);
    JZNode *toFunctionCall(asCScriptNode *node);
    
    JZNode *createOpNode(QString op);

    QString m_code;
    QString m_error;
    JZScriptFile *m_file;

    JZScriptItem *m_script;
};

#endif