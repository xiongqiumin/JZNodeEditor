#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include "JZModuleOpencvEditor.h"
#include "JZNodeParamDisplayWidget.h"
#include "CvToQt.h"
#include "JZNodeView.h"
#include "JZNodeEditorManager.h"
#include "JZEditorGlobal.h"

using namespace cv;

//JZOpencvTemplateDialog
JZOpencvTemplateDialog::JZOpencvTemplateDialog(QWidget *parent)
{
    m_label = new JZImageLabel();
    m_tempLabel = new JZImageLabel();
    m_propEditor = new JZPropertyEditor();

    QWidget *w = new QWidget();
    
    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->setContentsMargins(0,0,0,0);
    main_layout->addWidget(m_label);

    QVBoxLayout *r_layout = new QVBoxLayout();
    r_layout->setContentsMargins(0,0,0,0);
    r_layout->addWidget(new QLabel("模板"));
    r_layout->addWidget(m_tempLabel);
    r_layout->addWidget(new QLabel("匹配参数"));
    r_layout->addWidget(m_propEditor);
    main_layout->addLayout(r_layout);

    QPushButton *loadImageButton = m_btnBox->addButton("Load Image",QDialogButtonBox::ActionRole);
    QPushButton *loadTemplateButton = m_btnBox->addButton("Load Template",QDialogButtonBox::ActionRole);
    QPushButton *matchButton = m_btnBox->addButton("Match",QDialogButtonBox::ActionRole);

    connect(loadImageButton, &QPushButton::clicked, this, &JZOpencvTemplateDialog::on_loadImageButton_clicked);
    connect(loadTemplateButton, &QPushButton::clicked, this, &JZOpencvTemplateDialog::on_loadTemplateButton_clicked);
    connect(matchButton, &QPushButton::clicked, this, &JZOpencvTemplateDialog::on_matchButton_clicked);

    auto prop_group = m_propEditor->addGroup("属性");
    auto pin = m_propEditor->addProp("置信度", &m_config.confidence, prop_group);
    pin->setRange(0,1);

    w->setLayout(main_layout);
    setCentralWidget(w);
}

void JZOpencvTemplateDialog::setConfig(JZTemplateConfig cfg)
{
    m_config = cfg;
    m_propEditor->dataToUi();
}

JZTemplateConfig JZOpencvTemplateDialog::config() const
{
    m_propEditor->uiToData();
    return m_config;
}

void JZOpencvTemplateDialog::on_loadImageButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Image", "", "Image Files (*.png *.jpg *.bmp)");
    if (!filePath.isEmpty()) {
        m_image = cv::imread(filePath.toStdString());
        QImage image = QtOcv::mat2Image(m_image);
        m_label->setImage(image);
    }
}

void JZOpencvTemplateDialog::on_loadTemplateButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Template", "", "Image Files (*.png *.jpg *.bmp)");
    if (!filePath.isEmpty()) {
        m_templ = cv::imread(filePath.toStdString());
        QImage image = QtOcv::mat2Image(m_templ);
        m_tempLabel->setImage(image);

        m_config.templatePath = filePath;
    }
}

void JZOpencvTemplateDialog::on_matchButton_clicked()
{
    if (m_image.empty() || m_templ.empty()) {
        return;
    }
}    

//JZOpencvTemplateItem    
JZOpencvTemplateItem::JZOpencvTemplateItem(JZNode *node)
    :JZNodeGraphItem(node)
{
    m_setting = createButtonBlock("Setting", [this] { onSetClicked();  });
}

void JZOpencvTemplateItem::onSetClicked()
{
    JZNodeTemplateMatch *node = (JZNodeTemplateMatch *)m_node;
    JZOpencvTemplateDialog dlg(editor());
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
void JZModuleOpencvEditorInit()
{
    auto inst = editorManager()->instance();
    
    inst->registLogicNode(Node_OpencvTemplate, "模型", CreateJZNodeGraphItem<JZOpencvTemplateItem>);
}