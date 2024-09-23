#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleOpencv.h"
#include "JZNodeBind.h"
#include "JZNodeParamDelegate.h"
#include "CvMatAndQImage.h"

using namespace cv;

enum 
{
    Opencv_mat = 15000,
};

QVariant createMat(const QString &value)
{
    Mat *mat = new Mat();
    *mat = imread(qPrintable(value));
    return JZObjectCreateRefrence(mat, true);
}

QByteArray matPack(const QVariant &value)
{
    Mat *mat = JZObjectCast<Mat>(value);
    
    QImage image = QtOcv::mat2Image(*mat);
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return ba;
}

QVariant matUnpack(const QByteArray &buffer)
{
    QImage *image = new QImage();
    image->loadFromData(buffer);
    return JZObjectCreateRefrence(image, true);
}

//JZModuleOpencv
JZModuleOpencv::JZModuleOpencv()
{        
    m_name = "opencv";
}

JZModuleOpencv::~JZModuleOpencv()
{
}

void JZModuleOpencv::regist()
{
    auto func_inst = JZNodeFunctionManager::instance();

    int cls_id = Opencv_mat;
    jzbind::ClassBind<Mat> cls_mat(cls_id++, "Mat");
    cls_mat.setValueType(true);
    cls_mat.def("create", false, [](int col,int row){ return Mat(row,col,CV_8UC3); });
    cls_mat.def("resize", true, &Mat::clone);
    cls_mat.regist();
    m_classList << "Mat";
    
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
    m_functionList << "imread" << "threshold" << "medianBlur";

    auto d_inst = JZNodeParamDelegateManager::instance();

    JZNodeParamDelegate d_mat;
    d_mat.editType = Type_imageEdit;
    d_mat.createDisplay = CreateParamDisplayWidget<JZNodeImageDisplayWidget>;
    d_mat.createParam = createMat;    
    d_mat.pack = matPack;
    d_mat.unpack = matUnpack;
    d_inst->registDelegate(cls_mat.id(), d_mat);
}

void JZModuleOpencv::unregist()
{
    auto func_inst = JZNodeFunctionManager::instance();
    auto obj_inst = JZNodeObjectManager::instance();

    for(auto cls_name : m_classList)
        obj_inst->unregist(obj_inst->meta(cls_name)->id);

    for (auto func_id : m_functionList)
        func_inst->unregistFunction(func_id);

    m_classList.clear();
    m_functionList.clear();
}