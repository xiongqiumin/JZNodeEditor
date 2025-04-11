#ifndef JZ_ScriptItem_Help_H_
#define JZ_ScriptItem_Help_H_

#include "JZScriptItem.h"

class JZScriptItemHelp
{
public:
	JZScriptItemHelp(JZScriptItem* item);
	~JZScriptItemHelp();

	void addFunction(QString function,QStringList params);
	void addSet(QString dst, QString src);
	void addFor(QString start,int op,QString end);
	void addIf(QStringList condList);
	void addSwitch(QStringList condList);
	void addExpr(QString dst, int op,QString src1, QString src2);

protected:
	JZNode* getParamNode(QString param);
	JZNode* getSetParamNode(QString param);
	void pushSubFlow(JZNode *node);
	void popSubFlow();
	void switchSubFlow();

	JZScriptItem* m_script;
	QList<JZNode*> m_lastFlow;
	int m_subFlowIndex;
};

#endif // !JZScriptItemHelp
