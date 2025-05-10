#include "JZNodeFactory.h"
#include "JZModuleCameraEditor.h"
#include "JZModuleCamera.h"
#include "JZEditorGlobal.h"
#include "JZCameraNode.h"
#include "JZNodeView.h"

//JZCameraConfigDialog
JZCameraConfigDialog::JZCameraConfigDialog(QWidget *parent)
    :JZManagerPropertyDialog(parent)
{    
    auto browser = m_editor->browser();
    connect(browser, &JZPropertyBrowser::valueChanged, this, &JZCameraConfigDialog::onPropChanged);

    auto group = m_editor->addGroup("基本");    
    m_editor->addProp("名称", &m_config.name, group );

    QList<int> enmuList = { Camera_File, Camera_Hik };
    QStringList enumTextList = {"File" ,"Hik"};
    m_typeProp = m_editor->addPropIntEnum("类型", &m_config.type, enmuList, enumTextList, group);

    auto prop_group = m_editor->addGroup("属性");

    QList<JZProperty*> file_prop, hik_prop;
    //file
    file_prop << m_editor->addPropDir("路径", &m_config.filePath, prop_group);

    //hik
    hik_prop << m_editor->addProp("路径", &m_config.hikConfig.path,  prop_group);

    QStringList trigger_list = { "连续模式","触发模式" };
    hik_prop << m_editor->addPropIntEnum("触发模式", &m_config.hikConfig.triggerMode, { 0,1 }, trigger_list, prop_group);

    QList<int> trigger_source_list = { JZCamerHikConfig::TRIGGER_SOURCE_LINE0,
        JZCamerHikConfig::TRIGGER_SOURCE_LINE1,
        JZCamerHikConfig::TRIGGER_SOURCE_LINE2,
        JZCamerHikConfig::TRIGGER_SOURCE_LINE3,
        JZCamerHikConfig::TRIGGER_SOURCE_SOFTWARE, 
    };
    QStringList trigger_source_text_list = { "Line0","Line1","Line2","Line3","Software" };
    hik_prop << m_editor->addPropIntEnum("触发源", &m_config.hikConfig.triggerMode, trigger_source_list, trigger_source_text_list, prop_group);

    QStringList gain_list = { "关闭","一次", "连续" };
    hik_prop << m_editor->addPropIntEnum("增益模式", &m_config.hikConfig.gainMode, {0,1,2}, gain_list, prop_group);
    hik_prop << m_editor->addProp("增益", &m_config.hikConfig.gain, prop_group);

    QStringList exposure_list = { "关闭","一次", "连续" };
    hik_prop << m_editor->addPropIntEnum("曝光模式", &m_config.hikConfig.exposureMode, { 0,1,2 }, exposure_list, prop_group);
    hik_prop << m_editor->addProp("曝光", &m_config.hikConfig.exposureTime, prop_group);

    addPage(Camera_File, file_prop);
    addPage(Camera_Hik, hik_prop);
}

void JZCameraConfigDialog::setConfig(JZCameraConfig cfg)
{
    m_config = cfg;
    m_editor->dataToUi();
    switchPage(m_config.type);
}

JZCameraConfig JZCameraConfigDialog::getConfig() const
{
    return m_config;
}

void JZCameraConfigDialog::accept()
{
    m_editor->uiToData();
    JZManagerPropertyDialog::accept();
}

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
        camera_list << m_config.cameraList[i].name;

    JZCameraConfig cfg;
    cfg.name = JZRegExpHelp::uniqueString("camera", camera_list);
    cfg.type = Camera_File;

    JZCameraConfigDialog dlg(this);
    dlg.setConfig(cfg);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.cameraList << cfg;
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
        QTableWidgetItem *item = new QTableWidgetItem(cfg.name);
        m_table->setItem(i, 0, item);

        QTableWidgetItem *item_type = new QTableWidgetItem(m_camTypeList[cfg.type]);
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

    inst->registLogicNode(Node_CameraInit,"相机", CreateJZNodeGraphItem<JZCameraInitItem>);
    inst->registLogicNode(Node_CameraStart,"相机", CreateJZNodeGraphItem<JZCameraNodeItem>);
    inst->registLogicNode(Node_CameraStartOnce,"相机", CreateJZNodeGraphItem<JZCameraNodeItem>);
    inst->registLogicNode(Node_CameraStop,"相机", CreateJZNodeGraphItem<JZCameraNodeItem>);
    inst->registLogicNode(Node_CameraSetting,"相机", CreateJZNodeGraphItem<JZCameraNodeItem>);
    inst->registLogicNode(Node_CameraFrameReady,"相机", CreateJZNodeGraphItem<JZCameraNodeItem>);
}