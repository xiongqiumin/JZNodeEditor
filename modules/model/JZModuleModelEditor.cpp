#include "JZNodeFactory.h"
#include "JZModuleModelEditor.h"
#include "JZModelNode.h"
#include "JZEditorGlobal.h"
#include "JZRegExpHelp.h"
#include "JZNodeView.h"

//JZModelConfigDialog
JZModelConfigDialog::JZModelConfigDialog(QWidget *parent)
    :JZPropertyDialog(parent)
{
    auto browser = m_editor->browser();
    connect(browser, &JZPropertyBrowser::valueChanged, this, &JZModelConfigDialog::onPropChanged);

    auto group = m_editor->addGroup("基本");
    m_editor->addProp("名称", &m_name, group);

    QList<int> enmuList = { Model_Yolo };
    QStringList enumTextList = { "Yolo" };
    m_typeProp = m_editor->addPropIntEnum("类型", &m_type, enmuList, enumTextList, group);

    m_propGroup = m_editor->addGroup("属性");
    addYolo();    
}

void JZModelConfigDialog::addYolo()
{
    JZModelYoloConfig *yolo_cfg = new JZModelYoloConfig();
    m_config[Model_Yolo] = JZModelConfigPtr(yolo_cfg);

    QList<JZProperty*> model_yolo;
    model_yolo << m_editor->addPropFile("模型", &yolo_cfg->modelPath, "*.onnx", m_propGroup);
    model_yolo << m_editor->addPropFile("Meta", &yolo_cfg->idPath, "*.json", m_propGroup);

    addPage(Model_Yolo, model_yolo);
}

void JZModelConfigDialog::setConfig(JZModelConfigPtr cfg)
{
    m_name = cfg->name;
    m_type = cfg->type;
    JZModuleConfigFactory<JZModelConfig>::instance()->copyTo(cfg.data(), m_config[cfg->type].data());    
    m_editor->dataToUi();
    switchPage(m_type);
}

JZModelConfigPtr JZModelConfigDialog::getConfig() const
{
    JZModelConfigPtr ptr = m_config[m_type];
    ptr->name = m_name;
    return ptr;
}

void JZModelConfigDialog::accept()
{
    m_editor->uiToData();
    JZPropertyDialog::accept();
}

//JZModelInitDialog
JZModelInitDialog::JZModelInitDialog(QWidget *parent)
    :JZNodeManagerDialog(parent)
{
    QStringList strListHeader = { "名称", "类型" };
    m_table->setColumnCount(strListHeader.size());
    m_table->setHorizontalHeaderLabels(strListHeader);

    m_modelTypeList = QStringList{ "None","Yolo" };
}

void JZModelInitDialog::setConfig(JZModelManagerConfig cfg)
{
    m_config = cfg;
    updateConfig();
}

JZModelManagerConfig JZModelInitDialog::config()
{
    return m_config;
}

void JZModelInitDialog::addConfig()
{
    QStringList camera_list;
    for (int i = 0; i < m_config.modelList.size(); i++)
        camera_list << m_config.modelList[i]->name;

    JZModelYoloConfig *yolo_cfg = new JZModelYoloConfig();
    yolo_cfg->name = JZRegExpHelp::uniqueString("yolo", camera_list);
    
    JZModelConfigDialog dlg(this);
    dlg.setConfig(JZModelConfigPtr(yolo_cfg));
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.modelList << dlg.getConfig();
    updateConfig();
}

void JZModelInitDialog::removeConfig(int index)
{
    m_config.modelList.removeAt(index);
    updateConfig();
}

void JZModelInitDialog::settingConfig(int index)
{
    JZModelConfigDialog dlg(this);
    dlg.setConfig(m_config.modelList[index]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.modelList[index] = dlg.getConfig();
    updateConfig();
}

void JZModelInitDialog::updateConfig()
{
    m_table->setRowCount(m_config.modelList.size());

    QTableWidget *item = new QTableWidget();
    for (int i = 0; i < m_config.modelList.size(); i++)
    {
        auto &cfg = m_config.modelList[i];
        QTableWidgetItem *item = new QTableWidgetItem(cfg->name);
        m_table->setItem(i, 0, item);

        QTableWidgetItem *item_type = new QTableWidgetItem(m_modelTypeList[cfg->type]);
        m_table->setItem(i, 1, item_type);
    }
}

//JZModelInitItem    
JZModelInitItem::JZModelInitItem(JZNode *node)
    :JZNodeGraphItem(node)
{

}

void JZModelInitItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    if (!m_setting)
    {
        QPushButton *btnSet = new QPushButton("Setting");
        btnSet->connect(btnSet, &QPushButton::clicked, [this] {
            this->onSetClicked();
        });
        m_setting = createWidgetBlock(btnSet, true);
        m_setting->pri = 8;
    }
}

void JZModelInitItem::onSetClicked()
{
    JZNodeModelInit *node = (JZNodeModelInit *)m_node;
    JZModelInitDialog dlg(editor());
    dlg.setConfig(node->config());
    if (dlg.exec() != QDialog::Accepted)
        return;

    QByteArray oldValue = saveNode();
    node->setConfig(dlg.config());
    QByteArray newValue = saveNode();
    if (newValue == oldValue)
        return;

    notifyPropChanged(oldValue);
}

//JZModuleModelEditorInit
void JZModuleModelEditorInit()
{
    auto inst = editorManager()->instance();
    
    inst->registLogicNode(Node_ModelInit, "模型", QString(), CreateJZNodeGraphItem<JZModelInitItem>);
}