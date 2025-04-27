#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleOpencv.h"
#include "JZNodeBind.h"
#include "CvMatAndQImage.h"
#include "JZScriptEnvironment.h"
#include "JZYolo.h"
#include "JZYoloView.h"
#include "JZModelNode.h"
#include "JZNodeFactory.h"

using namespace cv;

QVariant createMat(JZScriptEnvironment *env,const QString &value)
{
    Mat *mat = new Mat();
    *mat = imread(qPrintable(value));
    return env->objectManager()->objectReferenceVariant(mat, true);
}

QByteArray matPack(JZScriptEnvironment *env, const QVariant &value)
{
    Mat *mat = env->objectManager()->objectCast<Mat>(value);
    
    QImage image = QtOcv::mat2Image(*mat);
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return ba;
}

QVariant matUnpack(JZScriptEnvironment *env, const QByteArray &buffer)
{
    QImage *image = new QImage();
    image->loadFromData(buffer);
    return env->objectManager()->objectReferenceVariant(image, true);
}

//JZModuleOpencv
JZModuleOpencv::JZModuleOpencv()
{        
    m_name = "opencv";

    m_classList << "Mat";
    m_functionList << "imread" << "threshold" << "medianBlur";
}

JZModuleOpencv::~JZModuleOpencv()
{
}

void JZModuleOpencv::regist(JZScriptEnvironment *env)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");

    auto func_inst = env->functionManager();
    int cls_id = Module_OpencvType;
    
    jzbind::ClassBind<Mat> cls_mat(cls_id++, "Mat");
    cls_mat.setValueType(true);
    cls_mat.def("create", false, [](int col,int row){ return Mat(row,col,CV_8UC3); });
    cls_mat.def("clone", true, &Mat::clone);
    cls_mat.regist();    
    
    func_inst->registCFunction("image2Mat", false, jzbind::createFuncion([](QImage image)->Mat {
        return QtOcv::image2Mat(image);
    }));
    func_inst->registCFunction("mat2Image", false, jzbind::createFuncion([](Mat mat) {
        return QtOcv::mat2Image(mat);
    }));


    func_inst->registCFunction("imread", false, jzbind::createFuncion([](QString file) {
        Mat out;
        out = imread(file.toLocal8Bit().data());
        return out;
    }));

    func_inst->registCFunction("threshold", false, jzbind::createFuncion([](Mat in, int thres){
        Mat out;
        threshold(in, out, thres, 255, THRESH_BINARY);
        return out;
    }));

    func_inst->registCFunction("medianBlur", false, jzbind::createFuncion([](Mat in, int size) {
        Mat out;
        medianBlur(in, out, size);
        return out;
    }));        

    jzbind::ClassBind<JZYoloResult> cls_yolo_ret(cls_id++, "JZYoloResult");
    cls_yolo_ret.regist();

    jzbind::ClassBind<QList<JZYoloResult>> cls_yolo_ret_list(cls_id++, "QList<JZYoloResult>");
    cls_yolo_ret_list.regist();

    jzbind::ClassBind<JZYolo> cls_yolo(cls_id++, "JZYolo");
    cls_yolo.def("loadNet", true, &JZYolo::loadNet);
    cls_yolo.def("forward", true, &JZYolo::forward);
    cls_yolo.regist();

    jzbind::ClassBind<JZYoloView> cls_yolo_view(cls_id++, "JZYoloView", "QWidget");
    cls_yolo_view.def("setYoloResult", true, &JZYoloView::setYoloResult);
    cls_yolo_view.regist();

    env->factoryManager()->registNode(Node_ModelForward, createJZNode<JZNodeModelForward>);    
}

void JZModuleOpencv::unregist(JZScriptEnvironment *env)
{
    auto func_inst = env->functionManager();
    auto obj_inst = env->objectManager();

    for(auto cls_name : m_classList)
        obj_inst->unregist(obj_inst->meta(cls_name)->id);

    for (auto func_id : m_functionList)
        func_inst->unregistFunction(func_id);

    m_classList.clear();
    m_functionList.clear();
}