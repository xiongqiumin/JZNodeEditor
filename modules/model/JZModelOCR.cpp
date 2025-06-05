#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include "JZModelOCR.h"
#include "../opencv/CvToQt.h"
#include "JZModelEngineFactory.h"

using namespace cv;

//JZModelOCRConfig
JZModelOCRConfig::JZModelOCRConfig()
{
    type = Model_OCR;
    backend = Model_BackendCpu;
    name = "ocr";
}

void JZModelOCRConfig::saveToStream(QDataStream& s) const
{
    JZModelConfig::saveToStream(s);
    s << backend;
    s << modelPath << idPath;
}

void JZModelOCRConfig::loadFromStream(QDataStream& s)
{
    JZModelConfig::loadFromStream(s);
    s >> backend;
    s >> modelPath >> idPath;
}

//JZModelOCRResult
QList<JZGraphic> JZModelOCRResult::toGraphics(const QList<JZModelOCRResult>& result)
{
    QList<JZGraphic> g_list;
    return g_list;
}

//JZModelOCR
JZModelOCR::JZModelOCR()
{
}

JZModelOCR::~JZModelOCR()
{
}

bool JZModelOCR::isInit()
{
    if (!m_net)
        return false;
    
    return m_net->isInit();
}

bool JZModelOCR::init()
{
    JZModelOCRConfig* cfg = dynamic_cast<JZModelOCRConfig*>(m_config.data());

    auto engine = JZModelEngineFactory::instance()->createEngine(cfg->backend);
    m_net = JZModelEnginePtr(engine);
    if (!m_net->load(cfg->modelPath))
        return false;

    return true;
}

void JZModelOCR::deinit()
{
    m_net.clear();
}

QList<JZModelOCRResult> JZModelOCR::forward(Mat frame)
{    
	QList<JZModelOCRResult>  result;

    return result;
}
