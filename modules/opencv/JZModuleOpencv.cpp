#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleOpencv.h"
#include "JZNodeBind.h"
#include "JZScriptEnvironment.h"
#include "JZOpencvNode.h"
#include "JZNodeFactory.h"
#include "JZContainer.h"
#include "CvToQt.h"
#include "jzWidgets/JZImageLabel.h"

using namespace cv;

//JZModuleOpencv
JZModuleOpencv::JZModuleOpencv()
{        
    m_name = "opencv";
}

JZModuleOpencv::~JZModuleOpencv()
{
}

void JZModuleOpencv::registCvtEnum(JZScriptEnvironment *env)
{
    auto obj_inst = env->objectManager();
    QStringList keyList = { "COLOR_BGR2GRAY",  "COLOR_GRAY2BGR" };
    QVector<int> valueList = { COLOR_BGR2GRAY,  COLOR_GRAY2BGR };

    JZNodeEnumDefine define;
    define.init("ColorConversionCodes",keyList, valueList);
    obj_inst->registEnum(define);
}

void JZModuleOpencv::regist(JZScriptEnvironment *env)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");

    auto obj_inst = env->objectManager();
    
    auto func_inst = env->functionManager();
    int cls_id = Module_OpencvType;
    
    jzbind::ClassBind<Mat> cls_mat(cls_id++, "Mat");
    cls_mat.setValueType(true);
    cls_mat.def("create", false, [](int col,int row){ return Mat(row,col,CV_8UC3); });
    cls_mat.def("clone", true, &Mat::clone);
    cls_mat.regist();    

    jzbind::ClassBind<Size> cls_size(cls_id++, "Size");
    cls_size.setValueType(true);
    cls_size.defProperty("width", &Size::width);
    cls_size.defProperty("height", &Size::height);
    cls_size.regist();

    jzbind::ClassBind<Size2d> cls_size_2d(cls_id++, "Size2d");
    cls_size_2d.setValueType(true);
    cls_size_2d.defProperty("width", &Size2d::height);
    cls_size_2d.defProperty("height", &Size2d::height);
    cls_size_2d.regist();

    jzbind::ClassBind<Point> cls_point(cls_id++, "Point");
    cls_point.setValueType(true);
    cls_point.defProperty("x", &Point::x);
    cls_point.defProperty("y", &Point::y);
    cls_point.regist();

    jzbind::ClassBind<Point2d> cls_point_2d(cls_id++, "Point2d");
    cls_point_2d.setValueType(true);
    cls_point_2d.defProperty("x", &Point2d::x);
    cls_point_2d.defProperty("y", &Point2d::y);
    cls_point_2d.regist();
    
    jzbind::ClassBind<Rect> cls_rect(cls_id++, "Rect");
    cls_rect.setValueType(true);
    cls_rect.defProperty("x", &Rect::x);
    cls_rect.defProperty("y", &Rect::y);
    cls_rect.defProperty("width", &Rect::width);
    cls_rect.defProperty("height", &Rect::height);
    cls_rect.regist();

    jzbind::ClassBind<Rect2d> cls_rect_2d(cls_id++, "Rect2d");
    cls_rect_2d.setValueType(true);
    cls_rect_2d.defProperty("x", &Rect2d::x);
    cls_rect_2d.defProperty("y", &Rect2d::y);
    cls_rect_2d.defProperty("width", &Rect2d::width);
    cls_rect_2d.defProperty("height", &Rect2d::height);
    cls_rect_2d.regist();

    jzbind::ClassBind<JZGraphic> cls_jz_graphic(cls_id++, "JZGraphic");
    cls_jz_graphic.regist();
    registList<JZGraphic>(env);

    jzbind::ClassBind<JZImageLabel> cls_jz_label(cls_id++, "JZImageLabel", "QWidget");
    cls_jz_label.def("setImage", true, &JZImageLabel::setImage);
    cls_jz_label.def("setGraphics", true, &JZImageLabel::setGraphics);
    cls_jz_label.def("clearGraphic", true, &JZImageLabel::clearGraphic);
    cls_jz_label.regist();
    
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

    auto cvt_func = func_inst->registCFunction("cvtColor", false, jzbind::createFuncion([](Mat mat, int type)->Mat {
        Mat out;
        cv::cvtColor(mat, out, type);
        return out;
    }));
    cvt_func->paramIn[0].type = "ColorConversionCodes";

    func_inst->registCFunction("threshold", false, jzbind::createFuncion([](Mat in, int thres){
        Mat out;
        threshold(in, out, thres, 255, THRESH_BINARY);
        return out;
    }));

    func_inst->registCFunction("blur", false, jzbind::createFuncion([](Mat in, Size size) {
        Mat out;
        blur(in, out, size);
        return out;
    }));

    func_inst->registCFunction("GaussianBlur", false, jzbind::createFuncion([](Mat in, Size size, double sigmaX,double sigmaY) {
        Mat out;
        GaussianBlur(in, out, size, sigmaX, sigmaY);
        return out;
    }));

    func_inst->registCFunction("medianBlur", false, jzbind::createFuncion([](Mat in, int size) {
        Mat out;
        medianBlur(in, out, size);
        return out;
    }));

    func_inst->registCFunction("bilateralFilter", false, jzbind::createFuncion([](Mat in, int d, double sigmaColor, double sigmaSpace) {
        Mat out;
        bilateralFilter(in, out, d, sigmaColor, sigmaSpace);
        return out;
    }));

    
    env->nodeFactory()->registNode(Node_OpencvInit, createJZNode<JZNodeOpencvInit>);

    //convert
    env->registConvert(cls_point.id(), Type_point, jzbind::createConvert<cv::Point,QPoint>(toQPoint));
    env->registConvert(cls_point_2d.id(), Type_pointF, jzbind::createConvert<cv::Point2d, QPointF>(toQPoint));
    env->registConvert(Type_point, cls_point_2d.id(), jzbind::createConvert<QPoint,cv::Point>(fromQPoint));
    env->registConvert(Type_pointF, cls_point_2d.id(), jzbind::createConvert<QPointF, cv::Point2d>(fromQPointF));

    env->registConvert(cls_size.id(), Type_size, jzbind::createConvert<cv::Size, QSize>(toQSize));
    env->registConvert(cls_size_2d.id(), Type_sizeF, jzbind::createConvert<cv::Size2d, QSizeF>(toQSizeF));
    env->registConvert(Type_size, cls_size_2d.id(), jzbind::createConvert<QSize, cv::Size>(fromQSize));
    env->registConvert(Type_sizeF, cls_size_2d.id(), jzbind::createConvert<QSizeF, cv::Size2d>(fromQSizeF));

    env->registConvert(cls_rect.id(), Type_rect, jzbind::createConvert<cv::Rect, QRect>(toQRect));
    env->registConvert(cls_rect_2d.id(), Type_rectF, jzbind::createConvert<cv::Rect2d, QRectF>(toQRectF));
    env->registConvert(Type_point, cls_rect_2d.id(), jzbind::createConvert<QRect, cv::Rect>(fromQRect));
    env->registConvert(Type_pointF, cls_rect_2d.id(), jzbind::createConvert<QRectF, cv::Rect2d>(fromQRectF));
}

void JZModuleOpencv::unregist(JZScriptEnvironment *env)
{
}