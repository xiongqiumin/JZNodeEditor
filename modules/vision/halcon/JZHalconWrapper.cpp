#include "halconWrapper.h"


HalconWrapper::HalconWrapper()
{
}



bool HalconWrapper::call(QString function,const QVariantList &in,const QVariantList &out)
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

HalconWrapper *HalconWrapper::createHalconWrapper()
{
	return new HalconWrapper();
}