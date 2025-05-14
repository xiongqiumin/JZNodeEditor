#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "JZYolo.h"
#include "../opencv/CvToQt.h"

//JZYolo
static const char *yolo_json = R"(
    {
  "architectures": [
    "Yolov8ForObjectDetection"
  ],
  "id2label": {
    "0": "person",
    "1": "bicycle",
    "2": "car",
    "3": "motorcycle",
    "4": "airplane",
    "5": "bus",
    "6": "train",
    "7": "truck",
    "8": "boat",
    "9": "traffic light",
    "10": "fire hydrant",
    "11": "stop sign",
    "12": "parking meter",
    "13": "bench",
    "14": "bird",
    "15": "cat",
    "16": "dog",
    "17": "horse",
    "18": "sheep",
    "19": "cow",
    "20": "elephant",
    "21": "bear",
    "22": "zebra",
    "23": "giraffe",
    "24": "backpack",
    "25": "umbrella",
    "26": "handbag",
    "27": "tie",
    "28": "suitcase",
    "29": "frisbee",
    "30": "skis",
    "31": "snowboard",
    "32": "sports ball",
    "33": "kite",
    "34": "baseball bat",
    "35": "baseball glove",
    "36": "skateboard",
    "37": "surfboard",
    "38": "tennis racket",
    "39": "bottle",
    "40": "wine glass",
    "41": "cup",
    "42": "fork",
    "43": "knife",
    "44": "spoon",
    "45": "bowl",
    "46": "banana",
    "47": "apple",
    "48": "sandwich",
    "49": "orange",
    "50": "broccoli",
    "51": "carrot",
    "52": "hot dog",
    "53": "pizza",
    "54": "donut",
    "55": "cake",
    "56": "chair",
    "57": "couch",
    "58": "potted plant",
    "59": "bed",
    "60": "dining table",
    "61": "toilet",
    "62": "tv",
    "63": "laptop",
    "64": "mouse",
    "65": "remote",
    "66": "keyboard",
    "67": "cell phone",
    "68": "microwave",
    "69": "oven",
    "70": "toaster",
    "71": "sink",
    "72": "refrigerator",
    "73": "book",
    "74": "clock",
    "75": "vase",
    "76": "scissors",
    "77": "teddy bear",
    "78": "hair drier",
    "79": "toothbrush"
},
  "layer_norm_eps": 0.001,
  "min_depth": 8,
  "num_channels": 3,
  "num_detection_tokens": 8400,
  "output_stride": 32,
  "semantic_loss_ignore_index": 255,
  "tf_padding": true,
  "model_type": "yolov8",
  "torch_dtype": "float32",
  "transformers_version": "4.33.3"
}
)";

//JZModelYoloConfig
JZModelYoloConfig::JZModelYoloConfig()
{

}

QDataStream& operator<<(QDataStream& s, const JZModelYoloConfig& param)
{
    s << param.modelPath << param.idPath;
    return s;
}

QDataStream& operator>>(QDataStream& s, JZModelYoloConfig& param)
{
    s >> param.modelPath >> param.idPath;
    return s;
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
    QJsonObject obj = QJsonDocument::fromJson(yolo_json).object();
    obj = obj["id2label"].toObject();
    auto it = obj.begin();
    while (it != obj.end())
    {
        int key = it.key().toInt();
        m_classList[key] = it.value().toString();
        it++;
    }
}

JZYolo::~JZYolo()
{
}

bool JZYolo::isVaild()
{
    return !m_net.empty();
}

bool JZYolo::loadNet(QString path)
{
    m_net = cv::dnn::readNet(path.toLocal8Bit().data());
    return true;
}

QList<JZYoloResult> JZYolo::forward(Mat frame)
{    
    float x_factor = frame.cols / 640.0f;
    float y_factor = frame.rows / 640.0f;

    // 推理
    cv::Mat blob = cv::dnn::blobFromImage(frame, 1 / 255.0, cv::Size(640, 640), cv::Scalar(0, 0, 0), true, false);
    m_net.setInput(blob);

    cv::Mat preds = m_net.forward();
    float confThreshold = 0.25f;
    float nmsThreshold = 0.45f;

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
