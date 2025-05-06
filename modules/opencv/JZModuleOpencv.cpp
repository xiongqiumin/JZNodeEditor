#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleOpencv.h"
#include "JZNodeBind.h"
#include "CvMatAndQImage.h"
#include "JZScriptEnvironment.h"
#include "JZOpencvNode.h"
#include "JZNodeFactory.h"
#include "JZContainer.h"

using namespace cv;

//JZModuleOpencv
JZModuleOpencv::JZModuleOpencv()
{        
    m_name = "opencv";
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

    env->nodeFactory()->registNode(Node_OpencvInit, createJZNode<JZNodeOpencvInit>);
    env->nodeFactory()->registNode(Node_OpencvTemplate, createJZNode<JZNodeTemplateMatch>);    
}

void JZModuleOpencv::unregist(JZScriptEnvironment *env)
{
}