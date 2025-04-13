#ifndef JZ_ScriptItem_Help_H_
#define JZ_ScriptItem_Help_H_

#include "JZScriptItem.h"
#include "JZScriptParser.h"

class JZScriptItemHelp
{
public:
	JZScriptItemHelp();
	~JZScriptItemHelp();

    void init(JZScriptItem* item);

    void addCall(QString function, QStringList params);
	void addSet(QString dst, QString src);
	void addFor(QString start,int op,QString end);
    void addWhile(QString cond);
	void addIf(QStringList condList);
	void addSwitch(QStringList condList, bool has_default);
	void addOperator(QString dst, int op,QString src1, QString src2);

protected:
    struct Flow{
        Flow();

        QList<JZNode*> nodes;
        int index;
    };    

    JZNode* addExpr(QString expr);
	JZNode* getParamNode(QString param);
	JZNode* getSetParamNode(QString param);
	void pushSubFlow(int sub_count);
	void popSubFlow();
	void switcbFlow(int idx);
    void nextFlow(JZNode *node);
    JZNode *lastFlow();    

	JZScriptItem* m_script;
	QList<Flow> m_flowStack;	
};

#endif // !JZScriptItemHelp
