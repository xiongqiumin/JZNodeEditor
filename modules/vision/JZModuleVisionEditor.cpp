#include <QSplitter>
#include <QFileDialog>
#include "JZModuleVisionEditor.h"
#include "JZVisionNode.h"
#include "JZEditorGlobal.h"
#include "JZNodeEditorManager.h"
#include "modules/opencv/CvToQt.h"
#include "JZVisionNode.h"

//JZVisionTemplateDialog
JZVisionTemplateDialog::JZVisionTemplateDialog(QWidget *parent)
{
    m_label = new JZImageLabel();
    m_tempLabel = new JZImageLabel();
    m_propEditor = new JZPropertyEditor();

    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *w = new QWidget();
    
    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->setContentsMargins(0,0,0,0);    

    QWidget *r_widget = new QWidget();
    QVBoxLayout *r_layout = new QVBoxLayout();
    r_layout->setContentsMargins(0,0,0,0);
    r_layout->addWidget(new QLabel("模板"));
    r_layout->addWidget(m_tempLabel);
    r_layout->addWidget(new QLabel("匹配参数"));
    r_layout->addWidget(m_propEditor);    
    r_widget->setLayout(r_layout);

    QSplitter *spiltter = new QSplitter();
    QPushButton *loadImageButton = m_btnBox->addButton("Load Image",QDialogButtonBox::YesRole);
    QPushButton *loadTemplateButton = m_btnBox->addButton("Load Template",QDialogButtonBox::YesRole);
    QPushButton *matchButton = m_btnBox->addButton("Match",QDialogButtonBox::YesRole);

    connect(loadImageButton, &QPushButton::clicked, this, &JZVisionTemplateDialog::on_loadImageButton_clicked);
    connect(loadTemplateButton, &QPushButton::clicked, this, &JZVisionTemplateDialog::on_loadTemplateButton_clicked);
    connect(matchButton, &QPushButton::clicked, this, &JZVisionTemplateDialog::on_matchButton_clicked);

    spiltter->addWidget(m_label);
    spiltter->addWidget(r_widget);
    spiltter->setChildrenCollapsible(false);
    spiltter->setSizes({ 600,200 });
    main_layout->addWidget(spiltter);

    auto prop_group = m_propEditor->addGroup("属性");
    auto pin = m_propEditor->addProp("置信度", &m_config.confidence, prop_group);
    pin->setRange(0,1);

    w->setLayout(main_layout);
    setCentralWidget(w);
    resize(800, 600);
}

void JZVisionTemplateDialog::setConfig(JZTemplateConfig cfg)
{
    m_config = cfg;
    m_propEditor->dataToUi();
}

JZTemplateConfig JZVisionTemplateDialog::config() const
{
    m_propEditor->uiToData();
    return m_config;
}

void JZVisionTemplateDialog::loadTemplate(QString filePath)
{
    m_templ = cv::imread(filePath.toStdString());
    QImage image = QtOcv::mat2Image(m_templ);
    m_tempLabel->setImage(image);
}

void JZVisionTemplateDialog::on_loadImageButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Image", "", "Image Files (*.png *.jpg *.bmp)");
    if (!filePath.isEmpty()) {
        m_image = cv::imread(filePath.toStdString());
        QImage image = QtOcv::mat2Image(m_image);
        m_label->setImage(image);
    }
}

void JZVisionTemplateDialog::on_loadTemplateButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Template", "", "Image Files (*.png *.jpg *.bmp)");
    if (filePath.isEmpty())
        return;

    m_config.templatePath = filePath;
    loadTemplate(filePath);        
}

void JZVisionTemplateDialog::on_matchButton_clicked()
{
    if (m_image.empty() || m_templ.empty()) {
        return;
    }

    m_temp.init(m_config);
    QRect rc = m_temp.match(m_image);
    if (rc.isValid())
    {
        m_label->clearGraphic();

        JZGraphic g = JZGraphic::fromRect(rc, Qt::red);        
        m_label->addGraphics(g);
    }

}    

//JZVisionTemplateItem    
JZVisionTemplateItem::JZVisionTemplateItem(JZNode *node)
    :JZNodeGraphItem(node)
{
    m_setting = createButtonBlock("Setting", [this] { onSetClicked();  });
}

void JZVisionTemplateItem::onSetClicked()
{
/*
    JZVisionTemplateMatch *node = (JZVisionTemplateMatch *)m_node;
    JZVisionTemplateDialog dlg(editor());
    dlg.setConfig(node->config());
    if (dlg.exec() != QDialog::Accepted)
        return;

    QByteArray oldValue = saveNode();
    node->setConfig(dlg.config());
    QByteArray newValue = saveNode();
    if (newValue == oldValue)
        return;

    notifyPropChanged(oldValue);
*/
}

//JZModuleVisionEditorInit
void JZModuleVisionEditorInit()
{
    auto inst = editorManager()->instance();

    auto icon = [](QString name)->QString {
        return ":/Modules/Vision/res/" + name;
    };
    inst->registLogicNode(Node_VisionCropImage, "Vision/图像处理", icon("crop.png"));
    inst->registLogicNode(Node_VisionImageFlip, "Vision/图像处理", icon("flip.png"));
    inst->registLogicNode(Node_VisionImageMorphology, "Vision/图像处理", icon("morphology.png"));
    inst->registLogicNode(Node_VisionImageRotate, "Vision/图像处理", icon("rotate_x.png"));
    inst->registLogicNode(Node_VisionImageSplice, "Vision/图像处理", icon("image_splice.png"));
    inst->registLogicNode(Node_VisionPerspectiveTransform, "Vision/图像处理", icon("perspective.png"));
    inst->registLogicNode(Node_VisionSkeleton, "Vision/图像处理", icon("skeleton.png"));

    inst->registLogicNode(Node_VisionBlobDetector, "Vision/检测识别", icon("blob.png"));
    inst->registLogicNode(Node_VisionBrightnessDetector, "Vision/检测识别", icon("brightness.png"));
    inst->registLogicNode(Node_VisionColorIdentify, "Vision/检测识别", icon("color_r.png"));

    inst->registLogicNode(Node_VisionShapeMatch, "Vision/对位工具", icon("shape_match.png"));
    inst->registLogicNode(Node_VisionTemplateMatch, "Vision/对位工具", icon("match.png"));

    inst->registLogicNode(Node_VisionFindCircle, "Vision/几何工具", icon("find_circle.png"));
    inst->registLogicNode(Node_VisionFindLine, "Vision/几何工具", icon("find_line.png"));
}