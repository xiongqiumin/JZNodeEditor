#include "JZHalconWrapper.h"


JZHalconWrapper::JZHalconWrapper()
{
}



bool JZHalconWrapper::call(QString function,const QVariantList &in,const QVariantList &out)
{
	try
	{
		
	}
	catch(const std::exception &e)
	{
		return false;
	}
	
	return true;
}

JZHalconWrapper *JZHalconWrapper::createJZHalconWrapper()
{
	return new JZHalconWrapper();
}