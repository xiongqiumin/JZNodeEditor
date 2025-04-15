#ifndef JZ_ScriptItem_Help_H_
#define JZ_ScriptItem_Help_H_

#include "JZScriptItem.h"

class asCScriptNode;
class JZScriptConvert
{
public:
	JZScriptConvert();
	~JZScriptConvert();

    void init(JZScriptItem* item);
	bool convertFunction(QString code);
	bool convertStatments(QString code);
    bool convertExpression(QString code);	

	QString error();

protected:
    struct Flow{
        Flow();

        QList<JZNode*> nodes;
        int index;
    };    

	template<class T>
	T* createNode() {
		T* node = new T();
		m_script->addNode(node);
		return node;
	}
	
	JZScriptEnvironment* environment();
	
	bool updateFunction(asCScriptNode* node);
	void nodeDebug(asCScriptNode* root, QString& result, int level);
	QString nodeDebug(asCScriptNode* node);
	void printNode(asCScriptNode* node);
	QString nodeText(asCScriptNode* node);
	asCScriptNode* nextNode(asCScriptNode* node, int count);
	QList<asCScriptNode*> nodeChilds(asCScriptNode* node);

	QList<JZParamDefine> toParamList(asCScriptNode* node);
	JZNode* toStatement(asCScriptNode* node);
	QList<JZNode*> toStatementBlock(asCScriptNode* node);

	JZNode* toReturn(asCScriptNode* node);
	JZNode* toIf(asCScriptNode* node);
	void setNodeIf(JZNodeIf* node_if, asCScriptNode* as_node, int cond);
	JZNode* toFor(asCScriptNode* node);
	JZNode* toWhile(asCScriptNode* node);
	JZNode* toSwitch(asCScriptNode* node);
	JZNode* toBreak(asCScriptNode* node);
	JZNode* toContinue(asCScriptNode* node);
	JZNode* toExpression(asCScriptNode* node);
	JZNode* toExprTerm(asCScriptNode* node);
	JZNode* toAssignment(asCScriptNode* node);
	JZNode* toFunctionCall(asCScriptNode* node);
    JZNode* toDeclaration(asCScriptNode* node);

	JZNode* createOpNode(QString op);

	JZScriptItem* m_script;
	QString m_code;
	QList<Flow> m_flowStack;
	QString m_error;
};

#endif // !JZScriptConvert
