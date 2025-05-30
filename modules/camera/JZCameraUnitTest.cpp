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

        m_depend->isTriggeScriptr = true;
        auto trigger_script = m_depend->triggerScript;
        auto trigger_start = trigger_script->startNode();

        JZCameraNode* camera_start = nullptr;
        if (m_depend->isRunOnce)
            camera_start = new JZNodeCameraStartOnce();
        else
            camera_start = new JZNodeCameraStart();

        camera_start->setCamera(camera_name);
        trigger_script->addNode(camera_start);
        trigger_script->addConnect(trigger_start->flowOutGemo(), camera_start->flowInGemo());
    }
}