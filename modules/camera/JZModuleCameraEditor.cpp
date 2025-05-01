#include "JZNodeFactory.h"
#include "JZModuleCameraEditor.h"
#include "JZModuleCamera.h"
#include "JZEditorGlobal.h"
#include "JZCameraNode.h"
#include "JZNodeView.h"

void JZCameraInitItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    if (!m_setting)
    {
        QPushButton *btnSet = new QPushButton("Add Cond");        
        btnSet->connect(btnSet, &QPushButton::clicked, [this] {
            this->onSetClicked();
        });
        m_setting = createWidgetBlock(btnSet,true);
        m_setting->pri = 8;        
    }    
}

void JZCameraInitItem::onSetClicked()
{
    JZNodeCameraInit *node = (JZNodeCameraInit *)m_node;
    JZCameraManagerConfig config = node->config();
    JZNodeManagerDialog dlg(editor());
    if(dlg.exec() != QDialog::Accepted)
        return;

    QByteArray oldValue = saveNode();
    node->setConfig(config);
    QByteArray newValue = saveNode();
    if(newValue == oldValue)
        return;

    notifyPropChanged(oldValue);
}

void JZCameraEditorInit()
{
    auto inst = editorManager()->instance();

    inst->registLogicNode(Node_CameraInit,"相机", CreateJZNodeGraphItem<JZCameraInitItem>);
    inst->registLogicNode(Node_CameraStart,"相机");
    inst->registLogicNode(Node_CameraStartOnce,"相机");
    inst->registLogicNode(Node_CameraStop,"相机");
    inst->registLogicNode(Node_CameraSetting,"相机");
    inst->registLogicNode(Node_CameraFrameReady,"相机");
}