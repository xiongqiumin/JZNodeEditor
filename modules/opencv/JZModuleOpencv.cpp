#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleOpencv.h"
#include "JZNodeBind.h"
#include "CvMatAndQImage.h"
#include "JZNodeParamDisplayWidget.h"
#include "JZScriptEnvironment.h"

using namespace cv;

enum 
{
    Opencv_mat = 15000,
};

QVariant createMat(JZScriptEnvironment *env,const QString &value)
{
    Mat *mat = new Mat();
    *mat = imread(qPrintable(value));
    return env->objectManager()->objectRefrence(mat, true);
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
    return env->objectManager()->objectRefrence(image, true);
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
    auto func_inst = env->functionManager();
    int cls_id = Opencv_mat;
    
    jzbind::ClassBind<Mat> cls_mat(cls_id++, "Mat");
    cls_mat.setValueType(true);
    cls_mat.def("create", false, [](int col,int row){ return Mat(row,col,CV_8UC3); });
    cls_mat.def("resize", true, &Mat::clone);
    cls_mat.regist();    
    
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

    auto d_inst = env->editorManager();

    JZNodeParamDelegate d_mat;
    d_mat.editType = Type_imageEdit;
    d_mat.createDisplay = CreateParamDisplayWidget<JZNodeImageDisplayWidget>;
    d_mat.createParam = createMat;    
    d_mat.pack = matPack;
    d_mat.unpack = matUnpack;
    d_inst->registDelegate(cls_mat.id(), d_mat);
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