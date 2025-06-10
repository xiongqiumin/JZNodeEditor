#include "JZModelEngineOnnx.h"
#include <QString>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include "JZModelEngineOnnxImpl.h"

// JZModelEngineOnnx 类实现
JZModelEngineOnnx::JZModelEngineOnnx(QObject *parent)
    : QObject(parent)
{
    d = new JZModelEngineOnnxImpl();
}

JZModelEngineOnnx::~JZModelEngineOnnx()
{
    delete d;
}

bool JZModelEngineOnnx::loadModel(const QString& modelPath, QString* errorMessage)
{
    return d->loadModel(modelPath, errorMessage);
}

cv::Mat JZModelEngineOnnx::forward(cv::Mat mat)
{
    return d->forward(mat);
}

QStringList JZModelEngineOnnx::getOutputNames() const
{
    return d->getOutputNames();
}

bool JZModelEngineOnnx::isModelLoaded() const
{    
    return d->isModelLoaded();
}