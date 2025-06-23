#include <inttypes.h>
#include <QCoreApplication>
#include "JZVision.h"
#include "modules/opencv/CvToQt.h"
#include "JZNodeEngine.h"

Mat JZVisionImageThreshold(Mat mat, QRect rc)
{
    return mat;
}

Mat JZVisionImageCrop(Mat mat, QRect rc) 
{
    auto cv_rc = fromQRect(rc);
	return mat(cv_rc);
}

Mat JZVisionImageFlip(Mat srcImage, bool h, bool v)
{
    Mat dstImage;
    if(!h && !v)    
        srcImage.copyTo(dstImage);
    else if(v && !h)
        cv::flip(srcImage, dstImage, 0);  //上下翻转
    else if (h && !v)
        cv::flip(srcImage, dstImage, 1);  //左右翻转		
    else
        cv::flip(srcImage, dstImage, -1);  //上下左右同时翻转			

	return dstImage;
}

void JZVisionImageMorphology() 
{
}

Mat JZVisionPerspectiveTransform(Mat mat, QRect from, QRect to)
{    
    // 定义源四边形顶点（顺时针顺序）
    std::vector<cv::Point2f> srcPoints;
    srcPoints.push_back(cv::Point2f(from.left(), from.top()));
    srcPoints.push_back(cv::Point2f(from.right(), from.top()));
    srcPoints.push_back(cv::Point2f(from.right(), from.bottom()));
    srcPoints.push_back(cv::Point2f(from.left(), from.bottom()));

    // 定义目标四边形顶点（顺时针顺序）
    std::vector<cv::Point2f> dstPoints;
    dstPoints.push_back(cv::Point2f(to.left(), to.top()));
    dstPoints.push_back(cv::Point2f(to.right(), to.top()));
    dstPoints.push_back(cv::Point2f(to.right(), to.bottom()));
    dstPoints.push_back(cv::Point2f(to.left(), to.bottom()));

    // 计算透视变换矩阵
    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);

    // 应用透视变换
    cv::Mat transformedMat;
    cv::warpPerspective(mat, transformedMat, perspectiveMatrix, mat.size());

    return transformedMat;
}

Mat JZVisionSkeleton(Mat src,int intera)
{
    Mat dst = src.clone();
    cv::threshold(dst, dst, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    
    int i, j, n;
    int width, height;
    width = src.cols - 1;
    //之所以减1，是方便处理8邻域，防止越界
    height = src.rows - 1;
    int step = src.step;
    int  p2, p3, p4, p5, p6, p7, p8, p9;
    uchar* img;
    bool ifEnd;
    int A1;
    cv::Mat tmpimg;
    //n表示迭代次数
    for (n = 0; n < intera; n++)
    {
        dst.copyTo(tmpimg);
        ifEnd = false;
        img = tmpimg.data;
        for (i = 1; i < height; i++)
        {
            img += step;
            for (j = 1; j < width; j++)
            {
                uchar* p = img + j;
                A1 = 0;
                if (p[0] > 0)
                {
                    if (p[-step] == 0 && p[-step + 1] > 0) //p2,p3 
                    {
                        A1++;
                    }
                    if (p[-step + 1] == 0 && p[1] > 0) //p3,p4 
                    {
                        A1++;
                    }
                    if (p[1] == 0 && p[step + 1] > 0) //p4,p5 
                    {
                        A1++;
                    }
                    if (p[step + 1] == 0 && p[step] > 0) //p5,p6 
                    {
                        A1++;
                    }
                    if (p[step] == 0 && p[step - 1] > 0) //p6,p7 
                    {
                        A1++;
                    }
                    if (p[step - 1] == 0 && p[-1] > 0) //p7,p8 
                    {
                        A1++;
                    }
                    if (p[-1] == 0 && p[-step - 1] > 0) //p8,p9 
                    {
                        A1++;
                    }
                    if (p[-step - 1] == 0 && p[-step] > 0) //p9,p2 
                    {
                        A1++;
                    }
                    p2 = p[-step] > 0 ? 1 : 0;
                    p3 = p[-step + 1] > 0 ? 1 : 0;
                    p4 = p[1] > 0 ? 1 : 0;
                    p5 = p[step + 1] > 0 ? 1 : 0;
                    p6 = p[step] > 0 ? 1 : 0;
                    p7 = p[step - 1] > 0 ? 1 : 0;
                    p8 = p[-1] > 0 ? 1 : 0;
                    p9 = p[-step - 1] > 0 ? 1 : 0;
                    if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) > 1 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) < 7 && A1 == 1)
                    {
                        if ((p2 == 0 || p4 == 0 || p6 == 0) && (p4 == 0 || p6 == 0 || p8 == 0)) //p2*p4*p6=0 && p4*p6*p8==0
                        {
                            dst.at<uchar>(i, j) = 0; //满足删除条件，设置当前像素为0
                            ifEnd = true;
                        }
                    }
                }
            }
        }
        dst.copyTo(tmpimg);
        img = tmpimg.data;
        for (i = 1; i < height; i++)
        {
            img += step;
            for (j = 1; j < width; j++)
            {
                A1 = 0;
                uchar* p = img + j;
                if (p[0] > 0)
                {
                    if (p[-step] == 0 && p[-step + 1] > 0) //p2,p3 
                    {
                        A1++;
                    }
                    if (p[-step + 1] == 0 && p[1] > 0) //p3,p4 
                    {
                        A1++;
                    }
                    if (p[1] == 0 && p[step + 1] > 0) //p4,p5 
                    {
                        A1++;
                    }
                    if (p[step + 1] == 0 && p[step] > 0) //p5,p6 
                    {
                        A1++;
                    }
                    if (p[step] == 0 && p[step - 1] > 0) //p6,p7 
                    {
                        A1++;
                    }
                    if (p[step - 1] == 0 && p[-1] > 0) //p7,p8 
                    {
                        A1++;
                    }
                    if (p[-1] == 0 && p[-step - 1] > 0) //p8,p9 
                    {
                        A1++;
                    }
                    if (p[-step - 1] == 0 && p[-step] > 0) //p9,p2 
                    {
                        A1++;
                    }
                    p2 = p[-step] > 0 ? 1 : 0;
                    p3 = p[-step + 1] > 0 ? 1 : 0;
                    p4 = p[1] > 0 ? 1 : 0;
                    p5 = p[step + 1] > 0 ? 1 : 0;
                    p6 = p[step] > 0 ? 1 : 0;
                    p7 = p[step - 1] > 0 ? 1 : 0;
                    p8 = p[-1] > 0 ? 1 : 0;
                    p9 = p[-step - 1] > 0 ? 1 : 0;
                    if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) > 1 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) < 7 && A1 == 1)
                    {
                        if ((p2 == 0 || p4 == 0 || p8 == 0) && (p2 == 0 || p6 == 0 || p8 == 0)) //p2*p4*p8=0 && p2*p6*p8==0
                        {
                            dst.at<uchar>(i, j) = 0; //满足删除条件，设置当前像素为0
                            ifEnd = true;
                        }
                    }
                }
            }
        }
        if (!ifEnd) break;
    }

    return dst;
}

std::vector<cv::KeyPoint> JZVisionBlobDetector(Mat src, cv::SimpleBlobDetector::Params param)
{
    cv::SimpleBlobDetector::Params params;
    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

    std::vector<cv::KeyPoint> keypoints;
    detector->detect(src, keypoints);
    return keypoints;
}

BrightnessDetectorResult JZVisionBrightnessDetector(Mat gary_img)
{
    double da;
    double cast;

    float a = 0;
    int Hist[256];
    for (int i = 0; i < 256; i++)
    {
        Hist[i] = 0;
    }
    for (int i = 0; i < gary_img.rows; i++)
    {
        for (int j = 0; j < gary_img.cols; j++)
        {
            a += float(gary_img.at<uchar>(i, j) - 128);
            int x = gary_img.at<uchar>(i, j);
            Hist[x]++;
        }
    }
    da = a / float(gary_img.rows * gary_img.cols);
    float D = abs(da);
    float Ma = 0;
    for (int i = 0; i < 256; i++)
    {
        Ma += abs(i - 128 - da) * Hist[i];
    }
    Ma /= float((gary_img.rows * gary_img.cols));
    float M = abs(Ma);
    float K = D / M;
    if (M == 0)
    {
        cast = 0;
    }
    else
    {
        cast = K;
    }

    BrightnessDetectorResult ret;
    ret.cast = cast;
    ret.da = da;
    return ret;
}

double JZVisionColorIdentify(Mat src_ori,Mat src_mat)
{    
    return 0;
}

void JZVisionFindCircle(Mat in)
{
}

void JZVisionFindLine(Mat in)
{
}

//JZBarCodeResult
QList<JZGraphic> JZBarCodeResult::toGraphics(const QList<JZBarCodeResult> &bar_list)
{
    QList<JZGraphic> list;
    for (int i = 0; i < bar_list.size(); i++)
    {
        JZGraphic g;
        g.type = JZGraphic::Polygon;
        g.color = Qt::green;
        for (auto pt : bar_list[i].points)
            g.points << pt;
        list.push_back(g);

        QPolygonF poly(g.points);
        auto rect = poly.boundingRect();

        JZGraphic g_text;
        g_text.type = JZGraphic::TextBox;
        g_text.color = Qt::green;
        g_text.text = bar_list[i].text;
        g_text.points << rect.topLeft() << rect.bottomRight();
        list.push_back(g_text);
    }
    return list;
}

JZBarCode::JZBarCode()
{
}

JZBarCode::~JZBarCode()
{
    deinit();
}

bool JZBarCode::isInit()
{
    return m_detector;
}

bool JZBarCode::init()
{
    if (m_detector)
        return true;

    QString model_dir = qApp->applicationDirPath() + "/model/wechat_qrcode";
    QString sr_prototxt = model_dir + "/sr.prototxt";
    QString sr_caffemodel = model_dir + "/sr.caffemodel";
    m_detector = cv::makePtr<cv::barcode::BarcodeDetector>(
        sr_prototxt.toStdString(),
        sr_caffemodel.toStdString());

    return true;
}

void JZBarCode::deinit()
{
    m_detector.reset();
}

QList<JZBarCodeResult> JZBarCode::detect(cv::Mat in)
{
    QList<JZBarCodeResult> ret_list;
         
    std::vector<cv::Point> vPoints;
    std::vector<std::string> decoded_text;
    std::vector<std::string> decoded_format;
    cv::Mat gray;
    cv::cvtColor(in, gray, cv::COLOR_BGR2GRAY);
    m_detector->detectAndDecodeWithType(gray, decoded_text, decoded_format, vPoints);
    for (int i = 0; i < decoded_text.size(); i++)
    {
        if (decoded_text[i].empty())
            continue;

        JZBarCodeResult ret;
        ret.type = QString::fromLocal8Bit(decoded_format[i].data());
        ret.text = QString::fromLocal8Bit(decoded_text[i].data());
        
        int pt_idx = 4 * i;
        for (int j = 0; j < 4; j++)
            ret.points << toQPoint(vPoints[pt_idx + j]);

        ret_list.push_back(ret);
    }

    return ret_list;
}

//JZQRCodeResult
QList<JZGraphic> JZQRCodeResult::toGraphics(const QList<JZQRCodeResult> &qr_list)
{
    QList<JZGraphic> list;
    for (int i = 0; i < qr_list.size(); i++)
    {
        JZGraphic g;
        g.type = JZGraphic::Polygon;
        g.color = Qt::green;        
        for(auto pt : qr_list[i].points)
            g.points << pt;
        list.push_back(g);

        QPolygonF poly(g.points);
        auto rect = poly.boundingRect();

        JZGraphic g_text;        
        g_text.type = JZGraphic::TextBox;
        g_text.color = Qt::green;
        g_text.text = qr_list[i].text;
        g_text.points << rect.topLeft() << rect.bottomRight();
        list.push_back(g_text);
    }
    return list;
}

//JZQRCode
JZQRCode::JZQRCode()
{
}

JZQRCode::~JZQRCode()
{
    deinit();
}

bool JZQRCode::isInit()
{
    return m_detector;
}

bool JZQRCode::init()
{
    if (m_detector)
        return true;

    QString model_dir = qApp->applicationDirPath() + "/model/wechat_qrcode";
    QString detect_prototxt = model_dir + "/detect.prototxt";
    QString detect_caffemodel = model_dir + "/detect.caffemodel";
    QString sr_prototxt = model_dir + "/sr.prototxt";
    QString sr_caffemodel = model_dir + "/sr.caffemodel";
    m_detector = cv::makePtr<cv::wechat_qrcode::WeChatQRCode>(
        detect_prototxt.toStdString(),
        detect_caffemodel.toStdString(),
        sr_prototxt.toStdString(),
        sr_caffemodel.toStdString());

    return true;
}

void JZQRCode::deinit()
{
    m_detector.reset();
}


QList<JZQRCodeResult> JZQRCode::detect(cv::Mat in)
{
    QList<JZQRCodeResult> ret_list;

    std::vector<cv::Mat> point_list;
    std::vector<std::string> strs = m_detector->detectAndDecode(in, point_list);
    for (int i = 0; i < strs.size(); i++)
    {
        JZQRCodeResult ret;
        ret.text = QString::fromLocal8Bit(strs[i].data());
        
        auto vPoints = point_list[i];
        for (int j = 0; j < 4; j++)
        {
            int x = (int)vPoints.at<float>(j, 0);
            int y = (int)vPoints.at<float>(j, 1);
            ret.points << QPoint(x,y);
        }

        ret_list.push_back(ret);
    }

    return ret_list;
}