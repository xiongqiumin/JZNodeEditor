#include <QApplication>
#include "JZPaddleOCR.h"
#include "modules/opencv/CvToQt.h"

using namespace PaddleOCR;

QList<JZGraphic> JZOCRResult::toGraphics(const QList<JZOCRResult> &result)
{
    QList<JZGraphic> g_list;
    for (int i = 0; i < result.size(); i++)
    {
        auto& ret = result[i];
        JZGraphic g;
        g.type = JZGraphic::TextBox;
        g.text = ret.text;      
        g.color = Qt::red;
        g.points << ret.rect.topLeft() << ret.rect.bottomRight();
        g_list.push_back(g);        
    }
    return g_list;
}

JZOCRResult::JZOCRResult()
{
    score = 0;
}

//JZPaddleOCR
JZPaddleOCR::JZPaddleOCR()
{
    m_init = false;
}

JZPaddleOCR::~JZPaddleOCR()
{
    deinit();
}

bool JZPaddleOCR::isInit()
{
    return m_init;
}

bool JZPaddleOCR::init()
{
    if (m_init)
        return true;

    QString mode_path = qApp->applicationDirPath() + "/model/paddle_ocr";
    std::string model_dir = mode_path.toLocal8Bit().data();
    m_detector.LoadModel(model_dir + "/ch_PP-OCRv4_det_infer.onnx");

    m_cls.LoadModel(model_dir + "/ch_ppocr_mobile_v2.0_cls_infer.onnx");

    m_rec.LoadLabel(model_dir + "/ppocr_keys_v1.txt");
    m_rec.LoadModel(model_dir + "/ch_PP-OCRv4_rec_infer.onnx");

    m_init = true;
    return true;
}

void JZPaddleOCR::deinit()
{
    m_init = false;
}

QList<JZOCRResult> JZPaddleOCR::ocr(cv::Mat mat)
{
    std::vector<std::vector<std::vector<int>>> boxes;
    std::vector<double> times;
    m_detector.Run(mat, boxes, times);  

    QList<JZOCRResult> orc_result;

    // °´det½á¹û£¬²ÃÇÐÍ¼Æ¬
    std::vector<cv::Mat> img_list;
    for (int j = 0; j < boxes.size(); j++)
    {
        cv::Mat crop_img;
        crop_img = Utility::GetRotateCropImage(mat, boxes[j]);
        img_list.push_back(crop_img);       

        auto box = boxes[j];
        int x_collect[4] = { box[0][0], box[1][0], box[2][0], box[3][0] };
        int y_collect[4] = { box[0][1], box[1][1], box[2][1], box[3][1] };
        int left = int(*std::min_element(x_collect, x_collect + 4));
        int right = int(*std::max_element(x_collect, x_collect + 4));
        int top = int(*std::min_element(y_collect, y_collect + 4));
        int bottom = int(*std::max_element(y_collect, y_collect + 4));

        JZOCRResult ret;
        ret.rect = QRect(QPoint(left, top), QPoint(right, bottom));
        orc_result.push_back(ret);
    }
    
    std::vector<int> cls_labels;
    std::vector<float> cls_scores;
    std::vector<double> cls_times;
    m_cls.Run(img_list, cls_labels, cls_scores, cls_times);

    float classifier_thresh = 0.9f;
    for (int i = 0; i < img_list.size(); i++)
    {
        if (cls_labels[i] % 2 == 1 && cls_scores[i] > classifier_thresh)
        {
            cv::rotate(img_list[i], img_list[i], 1);
        }
    }

    std::vector<std::string> rec_texts;
    std::vector<float> rec_text_scores;
    m_rec.Run(img_list, rec_texts, rec_text_scores, times);

    for (int i = 0; i < rec_texts.size(); i++)
    {
        std::cout << rec_texts[i] << std::endl;

        QString str = QString::fromUtf8(rec_texts[i].data());
        orc_result[i].text = str;
        orc_result[i].score = rec_text_scores[i];
    }

    QList<JZOCRResult> result;
    for (int i = 0; i < orc_result.size(); i++)
    {
        if (orc_result[i].score > 0.6)
            result << orc_result[i];
    }
    std::reverse(result.begin(), result.end());
    return result;
}