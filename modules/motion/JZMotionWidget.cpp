#include "JZMotionWidget.h"
#include "JZMotionMotean.h"
#include "JZRegExpHelp.h"
#include "../JZModuleConstant.h"

//JZMotionConfigDialog
JZMotionConfigDialog::JZMotionConfigDialog(QWidget *parent)
    :JZPropertyDialog(parent)
{
    auto group = m_editor->addGroup("基本");
    m_editor->addProp("名称", &m_name, group);

    QList<int> enmuList = { Motion_Motean };
    QStringList enumTextList = { "Motean" };
    m_typeProp = m_editor->addPropIntEnum("类型", &m_type, enmuList, enumTextList, group);

    m_propGroup = m_editor->addGroup("属性");    
    addMoteanPage();
}

JZMotionConfigDialog::~JZMotionConfigDialog()
{
    qDeleteAll(m_config);
}

void JZMotionConfigDialog::setConfig(JZMotionConfigEnum cfg)
{
    m_name = cfg->name;
    m_type = cfg->type;
    JZModuleConfigFactory<JZMotionConfig>::instance()->copyTo(cfg.data(), m_config[cfg->type]->data());    
    m_editor->dataToUi();
    switchPage(m_type);
}

JZMotionConfigEnum JZMotionConfigDialog::getConfig() const
{
    JZMotionConfigEnum ptr = *m_config[m_type];
    ptr->name = m_name;
    return ptr;
}

void JZMotionConfigDialog::addMoteanPage()
{
    auto c = JZModuleConstant::instance();

    JZMotionMoteanConfig *config = new JZMotionMoteanConfig();
    m_config[Motion_Motean] = new JZMotionConfigEnum(config);

    QList<JZProperty*> prop_list;
    prop_list << m_editor->addProp("PortName", &config->portName, m_propGroup);
    prop_list << m_editor->addPropIntEnum("Baud", &config->baud, c->BaudValue, c->BaudText, m_propGroup);
    prop_list << m_editor->addPropIntEnum("DataBit", &config->dataBit, c->DataBitValue, c->DataBitText, m_propGroup);
    prop_list << m_editor->addPropIntEnum("ParityBit", &config->parityBit, c->ParityBitValue, c->ParityBitText, m_propGroup);
    prop_list << m_editor->addPropIntEnum("StopBit", &config->stopBit, c->StopBitValue, c->StopBitText, m_propGroup);
        
    addPage(Motion_Motean, prop_list);
}

void JZMotionConfigDialog::accept()
{
    m_editor->uiToData();
    JZPropertyDialog::accept();
}

//JZMotionConfigWidget
JZMotionConfigWidget::JZMotionConfigWidget(QWidget* parent)
    :JZModuleConfigWidget(parent)
{
    QStringList strListHeader = { "名称", "类型" };
    m_table->setColumnCount(strListHeader.size());
    m_table->setHorizontalHeaderLabels(strListHeader);

    m_modelTypeList = QStringList{ "None","Yolo" };
}

void JZMotionConfigWidget::setConfig(JZMotionManagerConfig cfg)
{
    m_config = cfg;
    updateConfig();
}

JZMotionManagerConfig JZMotionConfigWidget::config()
{
    return m_config;
}

void JZMotionConfigWidget::addConfig()
{
    QStringList camera_list;
    for (int i = 0; i < m_config.motionList.size(); i++)
        camera_list << m_config.motionList[i]->name;

    JZMotionMoteanConfig* yolo_cfg = new JZMotionMoteanConfig();
    yolo_cfg->name = JZRegExpHelp::uniqueString("yolo", camera_list);

    JZMotionConfigDialog dlg(this);
    dlg.setConfig(JZMotionConfigEnum(yolo_cfg));
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.motionList << dlg.getConfig();
    updateConfig();
    emit sigModelChanged();
}

void JZMotionConfigWidget::removeConfig(int index)
{
    m_config.motionList.removeAt(index);
    updateConfig();
    emit sigModelChanged();
}

void JZMotionConfigWidget::settingConfig(int index)
{
    JZMotionConfigDialog dlg(this);
    dlg.setConfig(m_config.motionList[index]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.motionList[index] = dlg.getConfig();
    updateConfig();
    emit sigModelChanged();
}

void JZMotionConfigWidget::updateConfig()
{
    m_table->clearContents();
    m_table->setRowCount(m_config.motionList.size());

    QTableWidget* item = new QTableWidget();
    for (int i = 0; i < m_config.motionList.size(); i++)
    {
        auto& cfg = m_config.motionList[i];
        QTableWidgetItem* item = new QTableWidgetItem(cfg->name);
        m_table->setItem(i, 0, item);

        QTableWidgetItem* item_type = new QTableWidgetItem(m_modelTypeList[cfg->type]);
        m_table->setItem(i, 1, item_type);
    }
}