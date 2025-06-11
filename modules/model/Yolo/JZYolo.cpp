#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include "JZYolo.h"
#include "modules/opencv/CvToQt.h"
#include "../JZModelEngineFactory.h"

using namespace cv;

//JZModelYoloConfig
JZModelYoloConfig::JZModelYoloConfig()
{
    type = Model_Yolo;
    backend = Model_BackendCpu;
    name = "yolo";

    confThreshold = 0.7;
    nmsThreshold =  0.45;
}

void JZModelYoloConfig::saveToStream(QDataStream& s) const
{
    JZModelConfig::saveToStream(s);
    s << backend;
    s << modelPath << idPath;
    s << confThreshold;
    s << nmsThreshold;
}

void JZModelYoloConfig::loadFromStream(QDataStream& s)
{
    JZModelConfig::loadFromStream(s);
    s >> backend;
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
        JZGraphic g;
        g.type = JZGraphic::Rect;
        g.color = color_list[ret.id % 10];
        g.points << ret.rect.topLeft() << ret.rect.bottomRight();
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
    if (!m_net)
        return false;
    
    return m_net->isInit();
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
    JZModelYoloConfig* cfg = dynamic_cast<JZModelYoloConfig*>(m_config.data());
    if (!loadClassInfo(cfg->idPath))
        return false;

    auto engine = JZModelEngineFactory::instance()->createEngine(cfg->backend);
    m_net = JZModelEnginePtr(engine);
    if (!m_net->load(cfg->modelPath))
        return false;

    return true;
}

void JZYolo::deinit()
{
    m_net.clear();
}

cv::Mat JZYolo::makeLetterImage(const cv::Mat& img, cv::Size new_shape, LetterboxResult &box_result)
{
    cv::Mat ret;
    cv::Scalar color(114, 114, 114);

    // 获取原图尺寸
    cv::Size shape = img.size();

    // 计算缩放比例（保持原图长宽比）
    float scale = std::min((float)new_shape.height / (float)shape.height,
        (float)new_shape.width / (float)shape.width);

    // 计算新尺寸
    cv::Size new_unpad = cv::Size(
        std::round((float)shape.width * scale),
        std::round((float)shape.height * scale)
    );

    // 计算填充像素（左右上下对称填充）
    int dw = new_shape.width - new_unpad.width;
    int dh = new_shape.height - new_unpad.height;

    // 计算上下左右的填充量（对称分布）
    int pad_x = dw / 2;  // 左侧填充量
    int pad_y = dh / 2;  // 上侧填充量

    int right = dw - pad_x;  // 右侧填充量
    int bottom = dh - pad_y; // 下侧填充量

                             // 缩放图像
    cv::Mat resized;
    cv::resize(img, resized, new_unpad, 0, 0, cv::INTER_LINEAR);

    // 添加灰色填充（注意参数顺序：上、下、左、右）
    cv::copyMakeBorder(resized, ret,
        pad_y, bottom,
        pad_x, right,
        cv::BORDER_CONSTANT, color);

    box_result.pad_x = pad_x;
    box_result.pad_y = pad_y;
    box_result.scale = scale;
    return ret;
}

cv::Mat JZYolo::normalizedImage(cv::Mat img)
{    
    // BGR转RGB
    cv::Mat rgb;
    cv::cvtColor(img, rgb, cv::COLOR_BGR2RGB);

    // 归一化
    cv::Mat normalized;
    rgb.convertTo(normalized, CV_32F, 1.0 / 255.0);
    return normalized;
}

// 坐标映射函数：使用缩放比例和填充偏移量转换坐标
cv::Rect JZYolo::mapCoordinates(const cv::Rect& box, float scale, int pad_x, int pad_y) {
    float x1 = (box.x - pad_x) / scale;
    float y1 = (box.y - pad_y) / scale;
    float x2 = (box.x + box.width - pad_x) / scale;
    float y2 = (box.y + box.height - pad_y) / scale;

    // 确保坐标非负
    x1 = std::max(0.0f, x1);
    y1 = std::max(0.0f, y1);
    x2 = std::max(0.0f, x2);
    y2 = std::max(0.0f, y2);

    return cv::Rect(x1, y1, x2 - x1, y2 - y1);
}


QList<JZYoloResult> JZYolo::forward(Mat frame)
{    
    JZModelYoloConfig *cfg = dynamic_cast<JZModelYoloConfig*>(m_config.data());

    int inputH = 640;
    int inputW = 640;

    LetterboxResult letter_result;
    frame = makeLetterImage(frame, Size(inputW, inputH), letter_result);
    frame = normalizedImage(frame);    

    // 推理    
    cv::Mat preds = m_net->forward(frame);

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
        if (score > confThreshold)
        {
            float cx = det_output.at<float>(i, 0) * 640;
            float cy = det_output.at<float>(i, 1) * 640;
            float ow = det_output.at<float>(i, 2) * 640;
            float oh = det_output.at<float>(i, 3) * 640;
            int x = static_cast<int>((cx - 0.5 * ow));
            int y = static_cast<int>((cy - 0.5 * oh));
            int width = static_cast<int>(ow);
            int height = static_cast<int>(oh);

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
        box = mapCoordinates(box, letter_result.scale, letter_result.pad_x, letter_result.pad_y);

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
