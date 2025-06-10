#include "JZModelWidget.h"
#include "Yolo/JZYolo.h"
#include "JZRegExpHelp.h"

//JZModelConfigDialog
JZModelConfigDialog::JZModelConfigDialog(QWidget *parent)
    :JZPropertyDialog(parent)
{
    auto browser = m_editor->browser();    
    connect(browser, &JZPropertyBrowser::valueChanged, this, &JZModelConfigDialog::onModelBackendChanged);

    auto group = m_editor->addGroup("基本");
    m_editor->addProp("名称", &m_name, group);

    QList<int> enmuList = { Model_Yolo };
    QStringList enumTextList = { "Yolo" };
    m_typeProp = m_editor->addPropIntEnum("类型", &m_type, enmuList, enumTextList, group);

    m_propGroup = m_editor->addGroup("属性");
    addYolo();    
}

JZModelConfigDialog::~JZModelConfigDialog()
{
    qDeleteAll(m_config);
}

void JZModelConfigDialog::addYolo()
{
    JZModelYoloConfig *yolo_cfg = new JZModelYoloConfig();
    m_config[Model_Yolo] = new JZModelConfigEnum(yolo_cfg);
    
    QList<int> modelType = { Model_BackendCpu, Model_BackendTensorRT };
    QStringList modelTypeStr = { "cpu", "tensorRT" };

    QList<JZProperty*> model_yolo;
    m_yoloBackend = m_editor->addPropIntEnum("推理引擎", &yolo_cfg->backend, modelType, modelTypeStr, m_propGroup);
    model_yolo << m_yoloBackend;
    m_yoloModel = m_editor->addPropFile("模型", &yolo_cfg->modelPath, "*.onnx", m_propGroup);
    model_yolo << m_yoloModel;
    model_yolo << m_editor->addPropFile("Meta", &yolo_cfg->idPath, "*.json", m_propGroup);

    model_yolo << m_editor->addProp("置信度", &yolo_cfg->confThreshold, m_propGroup);
    model_yolo << m_editor->addProp("NMS阈值", &yolo_cfg->nmsThreshold, m_propGroup);

    addPage(Model_Yolo, model_yolo);
    updateModelFilter();
}

void JZModelConfigDialog::updateModelFilter()
{
    JZModelYoloConfig *yolo_cfg = dynamic_cast<JZModelYoloConfig *>(m_config[Model_Yolo]->data());

    JZPropertyFilePath *file_prop = dynamic_cast<JZPropertyFilePath*>(m_yoloModel);
    if (yolo_cfg->backend == Model_BackendTensorRT)
    {
        file_prop->setFileFilter("*.engine");
    }
    else
    {
        file_prop->setFileFilter("*.onnx");
    }
}

void JZModelConfigDialog::onModelBackendChanged(JZProperty * prop, const QVariant &v)
{
    updateModelFilter();
}

void JZModelConfigDialog::setConfig(JZModelConfigEnum cfg)
{
    m_name = cfg->name;
    m_type = cfg->type;
    JZModuleConfigFactory<JZModelConfig>::instance()->copyTo(cfg.data(), m_config[cfg->type]->data());    
    m_editor->dataToUi();
    switchPage(m_type);
}

JZModelConfigEnum JZModelConfigDialog::getConfig() const
{
    JZModelConfigEnum ptr = *m_config[m_type];
    ptr->name = m_name;
    return ptr;
}

void JZModelConfigDialog::accept()
{
    m_editor->uiToData();
    JZPropertyDialog::accept();
}

//JZModelConfigWidget
JZModelConfigWidget::JZModelConfigWidget(QWidget* parent)
    :JZModuleConfigWidget(parent)
{
    QStringList strListHeader = { "名称", "类型" };
    m_table->setColumnCount(strListHeader.size());
    m_table->setHorizontalHeaderLabels(strListHeader);

    m_modelTypeList = QStringList{ "None","Yolo" };
}

void JZModelConfigWidget::setConfig(JZModelManagerConfig cfg)
{
    m_config = cfg;
    updateConfig();
}

JZModelManagerConfig JZModelConfigWidget::config()
{
    return m_config;
}

void JZModelConfigWidget::addConfig()
{
    QStringList camera_list;
    for (int i = 0; i < m_config.modelList.size(); i++)
        camera_list << m_config.modelList[i]->name;

    JZModelYoloConfig* yolo_cfg = new JZModelYoloConfig();
    yolo_cfg->name = JZRegExpHelp::uniqueString("yolo", camera_list);

    JZModelConfigDialog dlg(this);
    dlg.setConfig(JZModelConfigEnum(yolo_cfg));
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.modelList << dlg.getConfig();
    updateConfig();
    emit sigModelChanged();
}

void JZModelConfigWidget::removeConfig(int index)
{
    m_config.modelList.removeAt(index);
    updateConfig();
    emit sigModelChanged();
}

void JZModelConfigWidget::settingConfig(int index)
{
    JZModelConfigDialog dlg(this);
    dlg.setConfig(m_config.modelList[index]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.modelList[index] = dlg.getConfig();
    updateConfig();
    emit sigModelChanged();
}

void JZModelConfigWidget::updateConfig()
{
    m_table->clearContents();
    m_table->setRowCount(m_config.modelList.size());

    QTableWidget* item = new QTableWidget();
    for (int i = 0; i < m_config.modelList.size(); i++)
    {
        auto& cfg = m_config.modelList[i];
        QTableWidgetItem* item = new QTableWidgetItem(cfg->name);
        m_table->setItem(i, 0, item);

        QTableWidgetItem* item_type = new QTableWidgetItem(m_modelTypeList[cfg->type]);
        m_table->setItem(i, 1, item_type);
    }
}