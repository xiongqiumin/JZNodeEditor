#include "JZNodeDebugClient.h"

JZNodeDebugClient::JZNodeDebugClient()
{
    connect(&m_client,&JZNetClient::sigConnect,this,&JZNodeDebugClient::onConnect);
	connect(&m_client,&JZNetClient::sigDisConnect,this,&JZNodeDebugClient::onDisConnect);
	connect(&m_client,&JZNetClient::sigNetPackRecv,this,&JZNodeDebugClient::onNetPackRecv);
}

JZNodeDebugClient::~JZNodeDebugClient()
{

}

bool JZNodeDebugClient::pause()
{
    return true;
}

bool JZNodeDebugClient::resume()
{
    return true;
}

bool JZNodeDebugClient::stop()
{
    return true;
}

void JZNodeDebugClient::onConnect()
{
    
}
	
void JZNodeDebugClient::onDisConnect()
{

}
	
void JZNodeDebugClient::onNetPackRecv(JZNetPackPtr ptr)
{

}