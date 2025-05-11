#include "JZNodeFactory.h"
#include "JZModuleModelEditor.h"
#include "JZModelNode.h"
#include "JZEditorGlobal.h"
#include "JZRegExpHelp.h"
#include "JZNodeView.h"

//JZModelConfigDialog
JZModelConfigDialog::JZModelConfigDialog(QWidget *parent)
    :JZManagerPropertyDialog(parent)
{
    auto browser = m_editor->browser();
    connect(browser, &JZPropertyBrowser::valueChanged, this, &JZModelConfigDialog::onPropChanged);

    auto group = m_editor->addGroup("基本");
    m_editor->addProp("名称", &m_config.name, group);

    QList<int> enmuList = { Model_Yolo };
    QStringList enumTextList = { "Yolo" };
    m_typeProp = m_editor->addPropIntEnum("类型", &m_config.type, enmuList, enumTextList, group);

    auto prop_group = m_editor->addGroup("属性");

    QList<JZProperty*> model_yolo;
    model_yolo << m_editor->addPropFile("模型", &m_config.modelPath, "*.onnx", prop_group);
    
    addPage(Model_Yolo, model_yolo);
}

void JZModelConfigDialog::setConfig(JZModelConfig cfg)
{
    m_config = cfg;
    m_editor->dataToUi();
    switchPage(m_config.type);
}

JZModelConfig JZModelConfigDialog::getConfig() const
{
    return m_config;
}

void JZModelConfigDialog::accept()
{
    m_editor->uiToData();
    JZManagerPropertyDialog::accept();
}

//JZModelInitDialog
JZModelInitDialog::JZModelInitDialog(QWidget *parent)
    :JZNodeManagerDialog(parent)
{
    QStringList strListHeader = { "名称", "类型" };
    m_table->setColumnCount(strListHeader.size());
    m_table->setHorizontalHeaderLabels(strListHeader);

    m_camTypeList = QStringList{ "None","Yolo" };
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
        camera_list << m_config.modelList[i].name;

    JZModelConfig cfg;
    cfg.name = JZRegExpHelp::uniqueString("model", camera_list);
    cfg.type = Model_Yolo;

    JZModelConfigDialog dlg(this);
    dlg.setConfig(cfg);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.modelList << cfg;
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
        QTableWidgetItem *item = new QTableWidgetItem(cfg.name);
        m_table->setItem(i, 0, item);

        QTableWidgetItem *item_type = new QTableWidgetItem(m_camTypeList[cfg.type]);
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