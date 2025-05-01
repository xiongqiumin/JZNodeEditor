#include "JZCameraUnitTest.h"
#include "JZCameraNode.h"

//JZNodeCameraVistor
JZNodeCameraVistor::JZNodeCameraVistor()
{

}

void JZNodeCameraVistor::visitorSelf(JZNode* node)
{
    if (node->type() == Node_CameraFrameReady)
    {
        JZNodeCameraReadyEvent* cam_event = (JZNodeCameraReadyEvent*)node;
        QString camera_name = cam_event->camera();

        auto class_item = node->file()->getClassItem();

        //trigger
        m_depend->isTrigger = true;
        auto trigger_script = m_depend->triggerScript;
        auto trigger_start = trigger_script->startNode();
        
        auto camera_start = new JZNodeCameraStartOnce();
        trigger_script->addNode(camera_start);
        camera_start->setCamera(camera_name);
        trigger_script->addConnect(trigger_start->flowOutGemo(), camera_start->flowInGemo());
    }
}