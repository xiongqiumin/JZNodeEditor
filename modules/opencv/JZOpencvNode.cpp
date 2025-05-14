#include "JZOpencvNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"

//JZNodeOpencvInit
JZNodeOpencvInit::JZNodeOpencvInit()
{
	m_type = Node_OpencvInit;
	m_name = "OpencvInit";
}
JZNodeOpencvInit::~JZNodeOpencvInit()
{
}

bool JZNodeOpencvInit::compiler(JZNodeCompiler* c, QString& error)
{
	return true;
}