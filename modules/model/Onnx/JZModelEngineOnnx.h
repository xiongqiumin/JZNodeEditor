#ifndef JZ_MODEL_ENGINE_ONNX_H
#define JZ_MODEL_ENGINE_ONNX_H

#include <QObject>
#include <QString>
#include <QVector>
#include <opencv2/opencv.hpp>

class JZModelEngineOnnxImpl;
class JZModelEngineOnnx : public QObject
{
    Q_OBJECT
public:
    explicit JZModelEngineOnnx(QObject *parent = nullptr);
    ~JZModelEngineOnnx();    

    // 加载模型
    bool loadModel(const QString& modelPath, QString* errorMessage = nullptr);

    // 执行推理
    cv::Mat forward(cv::Mat mat);

    // 获取模型信息
    QVector<int64_t> getInputDims() const;
    size_t getInputTensorSize() const;
    QStringList getOutputNames() const;

    // 判断模型是否已加载
    bool isModelLoaded() const;

private:    
    JZModelEngineOnnxImpl *d;
};

#endif // JZ_MODEL_ENGINE_ONNX_H