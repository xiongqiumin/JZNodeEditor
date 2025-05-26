#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include "JZYolo.h"
#include "../opencv/CvToQt.h"

//JZYolo

//JZModelYoloConfig
JZModelYoloConfig::JZModelYoloConfig()
{
    type = Model_Yolo;
    name = "yolo";

    confThreshold = 0.25;
    nmsThreshold =  0.45;
}

void JZModelYoloConfig::saveToStream(QDataStream& s) const
{
    JZModelConfig::saveToStream(s);
    s << modelPath << idPath;
    s << confThreshold;
    s << nmsThreshold;
}

void JZModelYoloConfig::loadFromStream(QDataStream& s)
{
    JZModelConfig::loadFromStream(s);
    s >> modelPath >> idPath;
    s >> confThreshold;
    s >> nmsThreshold;
}

//JZYoloResult
QList<JZGraphic> JZYoloResult::toGraphics(const QList<JZYoloResult>& result)
{
    QList<QColor> color_list;
    color_list.push_back(QColor(255, 0, 0));    // 蓝色
    color_list.push_back(QColor(0, 255, 0));    // 绿色
    color_list.push_back(QColor(0, 0, 255));   // 红色
    color_list.push_back(QColor(255, 255, 0));  // 青色
    color_list.push_back(QColor(255, 0, 255));  // 品红色
    color_list.push_back(QColor(0, 255, 255));  // 黄色
    color_list.push_back(QColor(128, 0, 0));    // 深蓝色
    color_list.push_back(QColor(0, 128, 0));    // 深绿色
    color_list.push_back(QColor(0, 0, 128));   // 深红色
    color_list.push_back(QColor(128, 128, 0));   // 深青色);

    QList<JZGraphic> g_list;
    for (int i = 0; i < result.size(); i++)
    {
        auto& ret = result[i];
        JZGraphic g = JZGraphic::fromRect(ret.rect, color_list[ret.id % 10]);
        g_list.push_back(g);
    }
    return g_list;
}

//JZYolo
JZYolo::JZYolo()
{
}

JZYolo::~JZYolo()
{
}

bool JZYolo::isInit()
{
    return !m_net.empty();
}

bool JZYolo::loadClassInfo(QString class_info)
{
    QFile file(class_info);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
    obj = obj["id2label"].toObject();
    auto it = obj.begin();
    while (it != obj.end())
    {
        int key = it.key().toInt();
        m_classList[key] = it.value().toString();
        it++;
    }
    return true;
}

bool JZYolo::init()
{
    JZModelYoloConfig *cfg = dynamic_cast<JZModelYoloConfig*>(m_config.data());
    if (!loadClassInfo(cfg->idPath))
        return false;
    
    try {
        m_net = cv::dnn::readNet(cfg->modelPath.toLocal8Bit().data());
    }
    catch (std::exception& e)
    {
        return false;
    }    
    return true;
}

QList<JZYoloResult> JZYolo::forward(Mat frame)
{    
    JZModelYoloConfig *cfg = dynamic_cast<JZModelYoloConfig*>(m_config.data());

    float x_factor = frame.cols / 640.0f;
    float y_factor = frame.rows / 640.0f;

    // 推理
    cv::Mat blob = cv::dnn::blobFromImage(frame, 1 / 255.0, cv::Size(640, 640), cv::Scalar(0, 0, 0), true, false);
    m_net.setInput(blob);

    cv::Mat preds = m_net.forward();
    float confThreshold = cfg->confThreshold;
    float nmsThreshold = cfg->nmsThreshold;

    QList<JZYoloResult> yolo_result;

    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    // 后处理, 1x84x8400
    cv::Mat outs(preds.size[1], preds.size[2], CV_32F, preds.ptr<float>());
    cv::Mat det_output = outs.t();
    
    for (int i = 0; i < det_output.rows; i++) 
    {
        cv::Mat classes_scores = det_output.row(i).colRange(4, preds.size[1]);
        cv::Point classIdPoint;

        double score;
        minMaxLoc(classes_scores, 0, &score, 0, &classIdPoint);

        // 置信度 0～1之间
        if (score > 0.25)
        {
            float cx = det_output.at<float>(i, 0) * 640;
            float cy = det_output.at<float>(i, 1) * 640;
            float ow = det_output.at<float>(i, 2) * 640;
            float oh = det_output.at<float>(i, 3) * 640;
            int x = static_cast<int>((cx - 0.5 * ow) * x_factor);
            int y = static_cast<int>((cy - 0.5 * oh) * y_factor);
            int width = static_cast<int>(ow * x_factor);
            int height = static_cast<int>(oh * y_factor);

            cv::Rect box;
            box.x = x;
            box.y = y;
            box.width = width;
            box.height = height;
            boxes.push_back(box);
            classIds.push_back(classIdPoint.x);
            confidences.push_back(score);
        }
    }

    std::vector<int> indices;
    // 进行非极大值抑制
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i) 
    {
        int idx = indices[i];

        cv::Rect box = boxes[idx];                        
        double confidence = confidences[idx];
        QString label = m_classList[classIds[idx]];
    
        JZYoloResult ret;
        ret.id = classIds[idx];
        ret.name = label;
        ret.rect = toQRect(box);
        ret.confidence = confidence;
        yolo_result.push_back(ret);
    }

    return yolo_result;
}
