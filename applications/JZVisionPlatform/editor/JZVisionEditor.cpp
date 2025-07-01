#include <QSplitter>
#include <QHBoxLayout>
#include <QUrlQuery>
#include "JZVisionEditor.h"
#include "modules/opencv/CvToQt.h"
#include "JZDockWidget.h"
#include "JZNodePanel.h"
#include "modules/camera/JZCameraNode.h"
#include "modules/communication/JZCommNode.h"
#include "modules/vision/JZVisionNode.h"
#include "modules/motion/JZMotionNode.h"
#include "modules/model/JZModelNode.h"

JZVisionEditor::JZVisionEditor(QWidget *parent) 
    : JZEditor(parent)
{
    m_type = ProjectItem_scriptItem;
    init();

    m_roiNode << Node_CameraFrameReady;
    
    m_roiNode << Node_VisionImageCrop;
    m_roiNode << Node_VisionImageFlip;    

    m_roiNode << Node_ModelYolo;
    m_roiNode << Node_visionAppBarCode;
    m_roiNode << Node_visionAppQrCode;
    m_roiNode << Node_visionAppOCR;
}

JZVisionEditor::~JZVisionEditor()
{
}

void JZVisionEditor::init()
{
    // 创建组件
    m_nodePanel = new JZVisionPanel();
    m_view = new JZVisionView(this);
    m_outputImage = new JZVisionImage(this);

    setFocusProxy(m_view);
    connect(m_view, &JZVisionView::redoAvailable, this, &JZVisionEditor::redoAvailable);
    connect(m_view, &JZVisionView::undoAvailable, this, &JZVisionEditor::undoAvailable);
    connect(m_view, &JZVisionView::modifyChanged, this, &JZVisionEditor::modifyChanged);    
    connect(m_view, &JZVisionView::sigAutoCompiler, this, &JZVisionEditor::sigAutoCompiler);    

    m_nodePanel->setView(m_view);
    m_view->setPanel(m_nodePanel);

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);    
    mainSplitter->addWidget(m_nodePanel);    
    mainSplitter->addWidget(m_view);
    mainSplitter->addWidget(new JZDockWidget("输出图像", m_outputImage));
    mainSplitter->setChildrenCollapsible(false);

    mainSplitter->setSizes({ 200,400,400 });
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(mainSplitter);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

JZVisionView* JZVisionEditor::view()
{
    return m_view;
}

void JZVisionEditor::setCompilerResult(const CompilerResult *info)
{
    m_view->setCompilerResult(info);
    updateOutputImage();
}

void JZVisionEditor::setRuntimeResult(int node_id, NodeResult result)
{
    m_result.nodeResult[node_id] = result;
    m_outputImage->setImage(node_id, result.outputImage[0]);
}

void JZVisionEditor::clearRuntimeResult()
{
    m_result.clear();
    m_outputImage->clear();
}

void JZVisionEditor::open(JZProjectItem *item)
{
    JZScriptItem* file = dynamic_cast<JZScriptItem*>(item);
    m_view->setFile(file);
    m_nodePanel->setFile(file);    
}

void JZVisionEditor::close()
{

}

void JZVisionEditor::save()
{
    m_view->save();
}

void JZVisionEditor::active()
{
    QMenu *menu = this->menu(Menu_View);
    if (!menu)
        return;

    m_actionList << menu->addSeparator();
    /*
    m_actionList << menu->addAction("自动布局");
    m_actionList << menu->addAction("显示全部");
    connect(m_actionList[1], &QAction::triggered, this, &JZVisionEditor::onActionLayout);
    connect(m_actionList[2], &QAction::triggered, this, &JZVisionEditor::onActionFitInView);
    */
}

void JZVisionEditor::inactive‌()
{
    QMenu *menu = this->menu(Menu_View);
    if (!menu)
        return;

    for (int i = 0; i < m_actionList.size(); i++)
    {
        menu->removeAction(m_actionList[i]);
        delete m_actionList[i];
    }
    m_actionList.clear();
}

bool JZVisionEditor::isModified()
{
    return m_view->isModified();
}

void JZVisionEditor::navigate(QUrl url)
{
    QUrlQuery query(url);
    int id = query.queryItemValue("id").toInt();
    m_view->selectNode(id);
}

void JZVisionEditor::updateOutputImage()
{
    auto script = m_view->file();

    auto node_id_list = script->nodeList();
    QList<JZVisionNodeInfo> node_list;
    for (int i = 0; i < node_id_list.size(); i++)
    {
        int node_id = node_id_list[i];
        JZVisionNodeInfo info;
        info.name = m_view->nodeName(node_id);
        info.node = script->getNode(node_id);
        info.hasImage = m_roiNode.contains(info.node->type());

        node_list << info;
    }

    m_outputImage->initNodeList(node_list);
}

void JZVisionEditor::undo()
{
    m_view->undo();
}

void JZVisionEditor::redo()
{
    m_view->redo();
}

void JZVisionEditor::remove()
{
    m_view->remove();
}

void JZVisionEditor::cut()
{
    m_view->cut();
}

void JZVisionEditor::copy()
{
    m_view->copy();
}

void JZVisionEditor::paste()
{
    m_view->paste();
}

void JZVisionEditor::selectAll()
{
    m_view->selectAll();
}