#ifndef JZ_ScriptItem_Help_H_
#define JZ_ScriptItem_Help_H_

#include "JZScriptItem.h"
#include "JZScriptFile.h"
#include "JZNodeFlow.h"
#include "JZNodeValue.h"
#include "JZNodeFunction.h"

class asCScriptNode;
class JZScriptConvert
{
public:
	JZScriptConvert();
	~JZScriptConvert();

	bool convertScript(QString code,JZScriptFile *file);
	bool convertFunction(QString code,JZScriptItem *script);
	bool convertStatments(QString code, JZScriptItem* script);
    bool convertExpression(QString code, JZScriptItem* script);

	QString error();

protected:
    struct BlockEnv
	{ 
		BlockEnv();

		QList<JZParamDefine> paramList;
		QList<JZNode*> flowList;
		
		JZNode* preStatment;
		JZNode* postStatment;
    };    
	typedef QSharedPointer<BlockEnv> BlockEnvPtr;

	template<class T>
	T* createNode() {
		T* node = new T();
		m_script->addNode(node);
		return node;
	}
	JZNodeParam* createGetParam(QString name);
	JZNodeSetParam* createSetParam(QString name);
	JZNode* createOpNode(QString op);
	JZNode* createSingleOpNode(QString op);
	JZNode* createObject(QString type);
	void visitNode(asCScriptNode* node, std::function<void(asCScriptNode*)> vistor);
	
	JZScriptEnvironment* environment();
	void init(JZScriptItem* script);
	JZNodeFunction *createFunction(QString function, asCScriptNode* node);
	
	bool addFunction(asCScriptNode* node);
	void nodeDebug(asCScriptNode* root, QString& result, int level);
	QString nodeDebug(asCScriptNode* node);
	void printNode(asCScriptNode* node);
	QString nodeText(asCScriptNode* node);
	asCScriptNode* nextNode(asCScriptNode* node, int count);
	QList<asCScriptNode*> nodeChilds(asCScriptNode* node);

	QString toDataType(asCScriptNode* node);
	QList<JZParamDefine> toParamList(asCScriptNode* node);
	bool toArgList(asCScriptNode* node, QList<JZNode*>& arg_list);

	JZNode* toExprTerm(asCScriptNode* node);
	JZNode* toExpression(asCScriptNode* node);
	JZNode* toAssignment(asCScriptNode* node);
	JZNode* toExpressionStatement(asCScriptNode* node);
	JZNode* toExpressionStatementFlow(asCScriptNode* node);
	JZNode* toFunctionCall(asCScriptNode* node);
	JZNode* toMemberFunctionCall(JZNode *self,asCScriptNode* node);

	const JZFunctionDefine *function(QString name);
	const JZParamDefine* getVariableInfo(QString name);
	void addLocalVariable(QString name, QString data_type);

	bool toStatement(asCScriptNode* node);
	bool toStatementBlock(asCScriptNode* node, QList<JZNode*>& list);

	bool toReturn(asCScriptNode* node);
	bool toIf(asCScriptNode* node);
	bool setNodeIf(JZNodeIf* node_if, asCScriptNode* as_node, int cond);
	bool toFor(asCScriptNode* node);
	bool toWhile(asCScriptNode* node);
	bool toSwitch(asCScriptNode* node);
	bool toBreak(asCScriptNode* node);
	bool toContinue(asCScriptNode* node);
	bool toFunctionCallStatement(asCScriptNode* node);
	bool toDeclarationStatement(asCScriptNode* node);
	
	BlockEnv* currentBlock();
	void pushBlock();
	void popBlock();

	JZScriptItem* m_script;
	QString m_code;
	QString m_error;
	QList<BlockEnvPtr> m_blockEnv;
	QMap<QString, JZParamDefine> m_localVaribaleMap;  //block变量到局部变量的转换
};

#endif // !JZScriptConvert
