#include <QApplication>
#include "mainwindow.h"
#include "JZNodeInit.h"
#include "JZModuleVisionApp.h"
#include <fstream>

// 日志记录器
class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        std::cout << "TensorRT Logger: " << msg << std::endl;
    }
};

// YOLO检测结果结构体
struct Detection {
    cv::Rect box;
    int classId;
    float confidence;
};

// 等比缩放并填充灰色边缘，同时返回缩放比例和填充偏移量
struct LetterboxResult
{
    float scale;       // 缩放比例
    int pad_x;         // X方向填充量
    int pad_y;         // Y方向填充量
};

cv::Mat makeLetterImage(const cv::Mat& img, cv::Size new_shape, LetterboxResult &box_result)
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

// 坐标映射函数：使用缩放比例和填充偏移量转换坐标
cv::Rect mapCoordinates(const cv::Rect& box, float scale, int pad_x, int pad_y) {
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

// 预处理图像
// 图像预处理函数
std::vector<float> preprocessImage(cv::Mat img, int inputH, int inputW)
{
    // 调整图像大小
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(inputW, inputH));

    // BGR转RGB
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);

    // 归一化
    cv::Mat normalized;
    rgb.convertTo(normalized, CV_32F, 1.0 / 255.0);

    // 转为CHW格式
    std::vector<float> inputData(inputH * inputW * 3);
    float* ptr = inputData.data();

    for (int c = 0; c < 3; ++c) {
        for (int h = 0; h < inputH; ++h) {
            for (int w = 0; w < inputW; ++w) {
                ptr[c * inputH * inputW + h * inputW + w] = normalized.at<cv::Vec3f>(h, w)[c];
            }
        }
    }

    return inputData;
}

// 后处理检测结果
std::vector<Detection> postprocess(const float* output, int outputSize, int inputW, int inputH, float confThreshold, float nmsThreshold)
{
    cv::Mat outs(84, 8400, CV_32F, (void*)output);
    cv::Mat det_output = outs.t();

    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    float x_factor = inputW / 640.0f;
    float y_factor = inputH / 640.0f;

    for (int i = 0; i < det_output.rows; i++)
    {
        cv::Mat classes_scores = det_output.row(i).colRange(4, 84);
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

    std::vector<Detection> yolo_result;

    std::vector<int> indices;
    // 进行非极大值抑制
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];

        cv::Rect box = boxes[idx];
        double confidence = confidences[idx];


        Detection ret;
        ret.box = box;
        ret.classId = 0;
        ret.confidence = confidence;
        yolo_result.push_back(ret);
    }

    return yolo_result;
}

// 绘制检测结果
void drawDetections(cv::Mat& image, const std::vector<Detection>& detections,
    const std::vector<std::string>& classNames) {
    for (const auto& det : detections) {
        // 绘制边界框
        cv::rectangle(image, det.box, cv::Scalar(0, 255, 0), 2);

        // 准备标签文本
        std::string label = classNames[det.classId] + ": " +
            std::to_string(static_cast<int>(det.confidence * 100)) + "%";

        // 绘制标签背景和文本
        int baseline = 0;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
        cv::rectangle(image,
            cv::Point(det.box.x, det.box.y - labelSize.height - baseline),
            cv::Point(det.box.x + labelSize.width, det.box.y),
            cv::Scalar(0, 255, 0), cv::FILLED);
        cv::putText(image, label,
            cv::Point(det.box.x, det.box.y - baseline),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
}

int main(int argc,char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    Q_INIT_RESOURCE(JZNodeEditor);
    Q_INIT_RESOURCE(vision);

    JZNodeInit();
    JZModuleManager::instance()->addModule(new JZModuleVisionApp());      

    MainWindow w;
    w.showMaximized();
    return a.exec();
}