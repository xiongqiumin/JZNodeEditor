#include "JZNodeFactory.h"
#include "JZModuleCameraEditor.h"
#include "JZModuleCamera.h"
#include "JZEditorGlobal.h"
#include "JZCameraNode.h"
#include "JZNodeView.h"

//JZCameraInitDialog
JZCameraInitDialog::JZCameraInitDialog(QWidget *parent)
    :JZNodeManagerDialog(parent)
{
    QStringList strListHeader = { "名称", "类型" };
    m_table->setColumnCount(strListHeader.size());
    m_table->setHorizontalHeaderLabels(strListHeader);

    m_camTypeList = QStringList{ "None","File","UVC","Hik" };
}

void JZCameraInitDialog::setConfig(JZCameraManagerConfig cfg)
{
    m_config = cfg;
    updateConfig();
}

JZCameraManagerConfig JZCameraInitDialog::config()
{
    return m_config;
}

void JZCameraInitDialog::addConfig() 
{
    QStringList camera_list;
    for (int i = 0; i < m_config.cameraList.size(); i++)
        camera_list << m_config.cameraList[i]->name;

    JZCameraFileConfig *cfg = new JZCameraFileConfig();
    cfg->name = JZRegExpHelp::uniqueString("camera", camera_list);    

    JZCameraConfigDialog dlg(this);
    dlg.setConfig(JZCameraConfigEnum(cfg));
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.cameraList << dlg.getConfig();
    updateConfig();
}

void JZCameraInitDialog::removeConfig(int index) 
{
    m_config.cameraList.removeAt(index);
    updateConfig();
}

void JZCameraInitDialog::settingConfig(int index) 
{
    JZCameraConfigDialog dlg(this);
    dlg.setConfig(m_config.cameraList[index]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.cameraList[index] = dlg.getConfig();
    updateConfig();
}

void JZCameraInitDialog::updateConfig()
{
    m_table->setRowCount(m_config.cameraList.size());

    QTableWidget *item = new QTableWidget();    
    for (int i = 0; i < m_config.cameraList.size(); i++)
    {
        auto &cfg = m_config.cameraList[i];
        QTableWidgetItem *item = new QTableWidgetItem(cfg->name);
        m_table->setItem(i, 0, item);

        QTableWidgetItem *item_type = new QTableWidgetItem(m_camTypeList[cfg->type]);
        m_table->setItem(i, 1, item_type);
    }
}

//JZCameraInitItem   
JZCameraInitItem::JZCameraInitItem(JZNode *node)
    :JZNodeGraphItem(node)
{

}


void JZCameraInitItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    if (!m_setting)
    {
        QPushButton *btnSet = new QPushButton("Setting");        
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
    JZCameraInitDialog dlg(editor());
    dlg.setConfig(node->config());
    if(dlg.exec() != QDialog::Accepted)
        return;

    QByteArray oldValue = saveNode();
    node->setConfig(dlg.config());
    QByteArray newValue = saveNode();
    if(newValue == oldValue)
        return;

    notifyPropChanged(oldValue);
}

//JZCameraNodeItem
JZCameraNodeItem::JZCameraNodeItem(JZNode *node)
    :JZNodeGraphItem(node)
{

}

void JZCameraNodeItem::updatePin()
{
    JZNodeGraphItem::updatePin();    
}

void JZCameraEditorInit()
{
    auto inst = editorManager()->instance();

    inst->registLogicNode(Node_CameraInit,"相机", QString(), CreateJZNodeGraphItem<JZCameraInitItem>);
    inst->registLogicNode(Node_CameraStart,"相机", QString(), CreateJZNodeGraphItem<JZCameraNodeItem>);
    inst->registLogicNode(Node_CameraStartOnce,"相机", QString(), CreateJZNodeGraphItem<JZCameraNodeItem>);
    inst->registLogicNode(Node_CameraStop,"相机", QString(), CreateJZNodeGraphItem<JZCameraNodeItem>);
    inst->registLogicNode(Node_CameraSetting,"相机", QString(), CreateJZNodeGraphItem<JZCameraNodeItem>);
    inst->registLogicNode(Node_CameraFrameReady,"相机", QString(), CreateJZNodeGraphItem<JZCameraNodeItem>);
}