#ifndef JZ_VISION_TABLE_STRING_TABLE
#define JZ_VISION_TABLE_STRING_TABLE

#include "JZNode.h"

class JZVisionStringTable
{
public:
	static JZVisionStringTable *instance();
	
	JZVisionStringTable();
	~JZVisionStringTable();
	
	QString nodeName(JZNode *node);
	
protected:
	QString functionName(JZNode* node, QString current_class);

	QMap<int,QString> m_nodeTypeMap;
};






#endif