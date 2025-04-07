#include "JZYolo.h"
#include "CvToQt.h"

JZYolo::JZYolo()
{
}

void JZYolo::loadNet(QString path)
{
    cv::dnn::Net net = cv::dnn::readNet(path);
    return true;
}

QList<YoloResult> JZYolo::forward(Mat mat)
{
    // 准备输入张量
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1.0 / 255.0, cv::Size(640, 640), cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);

    // 前向传播
    std::vector<cv::Mat> outs;
    std::vector<std::string> outNames = net.getUnconnectedOutLayersNames();
    net.forward(outs, outNames);

    // 后处理
    return postprocess(frame, outs, 0.25, 0.45);
}

// 对检测结果进行后处理
QList<YoloResult> JZYolo::postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outs, float confThreshold, float nmsThreshold) 
{
    QList<YoloResult> yolo_result;

    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (size_t i = 0; i < outs.size(); ++i) {
        // 解析检测输出
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
            cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            cv::Point classIdPoint;
            double confidence;
            // 找到最大置信度及其索引
            cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold) {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }

    std::vector<int> indices;
    // 进行非极大值抑制
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        cv::Rect box = boxes[idx];
        // 根据类别索引选择颜色
        cv::Scalar color = fixedColors[classIds[idx] % fixedColors.size()];
        // 绘制检测框
        cv::rectangle(frame, box, color, 2);
        // 绘制类别名称和置信度
        std::string label = cv::format("%.2f", confidences[idx]);
        if (!classNames.empty()) {
            CV_Assert(classIdPoint.x < (int)classNames.size());
            label = classNames[classIds[idx]] + ": " + label;
        }

        YoloResult ret;
        ret.name = label.c_str();
        ret.box = toQRect(box);
        yolo_result.puch_back(ret);
    }

    return yolo_result;
}
