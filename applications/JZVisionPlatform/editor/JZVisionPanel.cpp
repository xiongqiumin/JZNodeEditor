#include "JZVisionPanel.h"
#include "JZEditorGlobal.h"
#include "UiCommon.h"
#include "modules/camera/JZCameraNode.h"
#include "modules/model/JZModelNode.h"
#include "modules/communication/JZCommNode.h"
#include "modules/vision/JZVisionNode.h"
#include "modules/motion/JZMotionNode.h"
#include "JZContainer.h"
#include "JZNodeFunction.h"
#include "JZVisionAppNode.h"
#include "JZNodeAbstractView.h"

JZVisionPanel::JZVisionPanel()
{
    initLogicNode();
}

void JZVisionPanel::initLogicNode()
{
    auto icon = [](QString name)->QString {
        return ":/Modules/Vision/res/" + name;
    };

    //camera
    registLogicNode(Node_CameraFrameReady, "相机");
    registLogicNode(Node_CameraCalibration, "相机");
/*
    //vision
    registLogicNode(Node_VisionImageCrop, "图像处理", icon("crop.png"));
    registLogicNode(Node_VisionImageFlip, "图像处理", icon("flip.png"));
    registLogicNode(Node_VisionImageMorphology, "图像处理", icon("morphology.png"));
    registLogicNode(Node_VisionImageRotate, "图像处理", icon("rotate_x.png"));
    registLogicNode(Node_VisionImageSplice, "图像处理", icon("image_splice.png"));
    registLogicNode(Node_VisionPerspectiveTransform, "图像处理", icon("perspective.png"));
    registLogicNode(Node_VisionSkeleton, "图像处理", icon("skeleton.png"));

    registLogicNode(Node_VisionBlobDetector, "检测识别", icon("blob.png"));
    registLogicNode(Node_VisionBrightnessDetector, "检测识别", icon("brightness.png"));
    registLogicNode(Node_VisionColorIdentify, "检测识别", icon("color_r.png"));
*/
    registLogicNode(Node_visionAppBarCode, "识别");
    registLogicNode(Node_visionAppQrCode, "识别");
    registLogicNode(Node_visionAppOCR, "识别");
/*
    registLogicNode(Node_VisionShapeMatch, "对位工具", icon("shape_match.png"));
    registLogicNode(Node_VisionTemplateMatch, "对位工具", icon("match.png"));

    registLogicNode(Node_VisionFindCircle, "几何工具", icon("find_circle.png"));
    registLogicNode(Node_VisionFindLine, "几何工具", icon("find_line.png"));
*/
    //模型
    registLogicNode(Node_ModelYolo, "模型");

    //通信
    registLogicNode(Node_ModbusRead,  "通信");
    registLogicNode(Node_ModbusWrite, "通信");

    //运动
    registLogicNode(Node_MotionZero,  "运动");
    registLogicNode(Node_MotionMove,  "运动");
}

void JZVisionPanel::registLogicNode(int node_type, QString path, QString icon)
{
    JZLogicNode logic;
    logic.nodeType = node_type;
    logic.path = path;
    logic.icon = icon;
    m_logicList.push_back(logic);
}

void JZVisionPanel::init()
{
    QTreeWidgetItem *item_basic = createFolder("基本");
    m_tree->addTopLevelItem(item_basic);
    
    initLocalParam(item_basic);
    initProcess(item_basic);
    initExpression(item_basic,true);
    initContainer(item_basic);

    initLogic(m_tree->invisibleRootItem());
}

void JZVisionPanel::updateDefine()
{
    updateLocalParam();
}

QTreeWidgetItem *JZVisionPanel::createDefaultNode(JZNode *node)
{
    auto env = m_view->file()->project()->environment();

    auto in_list = node->paramInList();
    for (int i = 0; i < in_list.size(); i++)
    {
        auto pin = node->pin(in_list[i]);
        if (pin->flag() & Pin_noCompiler)
            continue;

        auto pin_type = pin->dataType();
        auto up_type = env->upType(pin_type);
        auto type_id = env->nameToType(up_type);
        if (JZNodeType::isBaseOrEnum(type_id))
        {
            QString value = env->defaultValueString(type_id);
            node->setPinValue(in_list[i],value);
        }
    }

    return createNode(node);
}

void JZVisionPanel::initContainer(QTreeWidgetItem *item_root)
{
    QTreeWidgetItem *item_container = createFolder("容器");
    item_root->addChild(item_container);

    QTreeWidgetItem *item_list = createFolder("List");
    item_container->addChild(item_list);

    QTreeWidgetItem *item_map = createFolder("Map");
    item_container->addChild(item_map);

    auto funciont_list = JZContainerManager::instance()->functionList();
    for (int i = 0; i < funciont_list.size(); i++)
    {
        auto func_name = funciont_list[i];
        auto func_def = JZContainerManager::instance()->function(func_name);

        auto coor = JZFunctionHelper::splitFunction(func_name);

        JZNodeContainerFunction func_node;
        func_node.setFunction(func_name);
        func_node.setForceFlow(true);

        QTreeWidgetItem *item = createDefaultNode(&func_node);
        item->setText(0, coor.name);

        if (func_name.startsWith("QList<"))
        {
            item_list->addChild(item);
        }
        else if (func_name.startsWith("QMap<"))
        {
            item_map->addChild(item);
        }
    }    
}

void JZVisionPanel::initLogic(QTreeWidgetItem *item_root)
{
    auto logic_list = m_logicList;
    for (int i = 0; i < logic_list.size(); i++)
    {
        auto &node = logic_list[i];

        QStringList path = node.path.split("/");
        QTreeWidgetItem *item = item_root;
        for (int i = 0; i < path.size(); i++)
        {
            int sub_idx = UiHelper::treeIndexOf(item, path[i]);
            if (sub_idx >= 0)
                item = item->child(sub_idx);
            else
            {
                auto sub_item = createFolder(path[i]);
                item->addChild(sub_item);
                item = sub_item;
            }
        }

        auto jznode = editorNodeFactory()->createNode(node.nodeType);
        auto sub_item = createDefaultNode(jznode);
        if (!node.icon.isEmpty())
            sub_item->setIcon(0, QIcon(node.icon));

        item->addChild(sub_item);
        delete jznode;
    }
}