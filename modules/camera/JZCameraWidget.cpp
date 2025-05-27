#include "JZCameraWidget.h"
#include "JZCameraHik.h"
#include "JZCameraFile.h"
#include "JZCameraUvc.h"
#include "JZCameraRtsp.h"
#include "../JZModuleConfigFactory.h"

//JZCameraConfigDialog
JZCameraConfigDialog::JZCameraConfigDialog(QWidget *parent)
    :JZPropertyDialog(parent)
{          
    auto browser = m_editor->browser();
    connect(browser, &JZPropertyBrowser::valueChanged, this, &JZCameraConfigDialog::onPropChanged);

    auto group = m_editor->addGroup("基本");
    m_editor->addProp("名称", &m_name, group);

    QList<int> enmuList = { Camera_File, Camera_Hik, Camera_Rtsp };
    QStringList enumTextList = { "File" ,"Hik" , "Rtsp" };
    m_typeProp = m_editor->addPropIntEnum("类型", &m_type, enmuList, enumTextList, group);

    m_propGroup = m_editor->addGroup("属性");
    m_type = Camera_File;

    addFilePage();
    addHikPage();
    addUvcPage();
    addRtspPage();
}

JZCameraConfigDialog::~JZCameraConfigDialog()
{
    qDeleteAll(m_config);
}

void JZCameraConfigDialog::addFilePage()
{
    JZCameraFileConfig *cfg_file = new JZCameraFileConfig();
    m_config[Camera_File] = new JZCameraConfigEnum(cfg_file);

    QList<JZProperty*> file_prop;
    file_prop << m_editor->addPropDir("路径", &cfg_file->path, m_propGroup);

    addPage(Camera_File, file_prop);
}

void JZCameraConfigDialog::addHikPage()
{
    JZCameraHikConfig *cfg_hik = new JZCameraHikConfig();
    m_config[Camera_Hik] = new JZCameraConfigEnum(cfg_hik);

    QList<JZProperty*> hik_prop;
    //hik
    hik_prop << m_editor->addProp("路径", &cfg_hik->path, m_propGroup);

    QStringList trigger_list = { "连续模式","触发模式" };
    hik_prop << m_editor->addPropIntEnum("触发模式", &cfg_hik->triggerMode, { 0,1 }, trigger_list, m_propGroup);

    QList<int> trigger_source_list = { JZCameraHikConfig::TRIGGER_SOURCE_LINE0,
        JZCameraHikConfig::TRIGGER_SOURCE_LINE1,
        JZCameraHikConfig::TRIGGER_SOURCE_LINE2,
        JZCameraHikConfig::TRIGGER_SOURCE_LINE3,
        JZCameraHikConfig::TRIGGER_SOURCE_SOFTWARE,
    };
    QStringList trigger_source_text_list = { "Line0","Line1","Line2","Line3","Software" };
    hik_prop << m_editor->addPropIntEnum("触发源", &cfg_hik->triggerMode, trigger_source_list, trigger_source_text_list, m_propGroup);

    QStringList gain_list = { "关闭","一次", "连续" };
    hik_prop << m_editor->addPropIntEnum("增益模式", &cfg_hik->gainMode, { 0,1,2 }, gain_list, m_propGroup);
    hik_prop << m_editor->addProp("增益", &cfg_hik->gain, m_propGroup);

    QStringList exposure_list = { "关闭","一次", "连续" };
    hik_prop << m_editor->addPropIntEnum("曝光模式", &cfg_hik->exposureMode, { 0,1,2 }, exposure_list, m_propGroup);
    hik_prop << m_editor->addProp("曝光", &cfg_hik->exposureTime, m_propGroup);

    addPage(Camera_Hik, hik_prop);
}

void JZCameraConfigDialog::addUvcPage()
{
    JZCameraUvcConfig *cfg_uvc = new JZCameraUvcConfig();
    m_config[Camera_UVC] = new JZCameraConfigEnum(cfg_uvc);

    //addPage(Camera_UVC, uvc_prop);
}

void JZCameraConfigDialog::addRtspPage()
{
    JZCameraRtspConfig *cfg_rtsp = new JZCameraRtspConfig();
    m_config[Camera_Rtsp] = new JZCameraConfigEnum(cfg_rtsp);

    QList<JZProperty*> rtsp_prop;
    rtsp_prop << m_editor->addProp("Url", &cfg_rtsp->path, m_propGroup);

    addPage(Camera_Rtsp, rtsp_prop);
}

void JZCameraConfigDialog::setConfig(JZCameraConfigEnum cfg)
{
    Q_ASSERT(m_config.contains(cfg->type));
    
    m_name = cfg->name;
    m_type = cfg->type;
    JZModuleConfigFactory<JZCameraConfig>::instance()->copyTo(cfg.data(), m_config[cfg->type]->data());
    m_editor->dataToUi();
    switchPage(cfg->type);
}

JZCameraConfigEnum JZCameraConfigDialog::getConfig() const
{
    JZCameraConfigEnum ptr = *m_config[m_type];
    ptr->name = m_name;
    return ptr;
}

void JZCameraConfigDialog::accept()
{
    m_editor->uiToData();
    JZPropertyDialog::accept();
}