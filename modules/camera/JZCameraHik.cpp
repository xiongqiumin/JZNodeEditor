#include <QUrl>
#include <QThread>
#include <QJsonParseError>
#include <QJsonObject>
#include "JZCameraHik.h"
#include "E:\libs\MVS\Development\Includes\MvCameraControl.h"

class JZCameraHikApi
{
public:
    static JZCameraHikApi *instance();

    bool enumDevices();

    MV_CC_DEVICE_INFO_LIST  m_stDevList;

protected:
    JZCameraHikApi();
    ~JZCameraHikApi();        
};

JZCameraHikApi *JZCameraHikApi::instance()
{
    static JZCameraHikApi inst;
    return &inst;
}

JZCameraHikApi::JZCameraHikApi()
{
}

JZCameraHikApi::~JZCameraHikApi()
{
}

bool JZCameraHikApi::enumDevices()
{
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // ch:枚举子网内所有设备 | en:Enumerate all devices within subnet
    int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_GIGE_DEVICE | MV_GENTL_CAMERALINK_DEVICE |
        MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &m_stDevList);
    return nRet == MV_OK;    
}

#define g_api JZCameraHikApi::instance()

//JZCameraHik
JZCameraHik::JZCameraHik()
{
    m_hDevHandle = nullptr;
    m_thread = nullptr;
    m_isStartGrabbing = false;
}

JZCameraHik::~JZCameraHik()
{
    close();
}

bool JZCameraHik::isOpen()
{
    return m_hDevHandle != nullptr;
}

bool JZCameraHik::open(QString path)
{
    g_api->enumDevices();
    
    MV_CC_DEVICE_INFO* pstDeviceInfo = nullptr;
    for (int i = 0; i < g_api->m_stDevList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = g_api->m_stDevList.pDeviceInfo[i];
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            int nIp1 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int nIp2 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int nIp3 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int nIp4 = (pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
            QString device_ip = QString::asprintf("%d.%d.%d.%d", nIp1, nIp2, nIp3, nIp4);
            
            if (QUrl(path) == QUrl(device_ip))
            {
                pstDeviceInfo = pDeviceInfo;
                break;
            }
        }
        else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {

        }
    }
    if (!pstDeviceInfo)
        return false;

    int nRet = MV_CC_CreateHandle(&m_hDevHandle, pstDeviceInfo);
    if (MV_OK != nRet)
    {
        return nRet;
    }
    
    nRet = MV_CC_OpenDevice(m_hDevHandle, MV_ACCESS_Exclusive,0);
    if (MV_OK != nRet)
    {
        MV_CC_DestroyHandle(m_hDevHandle);
        m_hDevHandle = nullptr;
        return false;
    }

    return true;
}

void JZCameraHik::close()
{
    if (!m_hDevHandle)
        return;

    stop();

    MV_CC_CloseDevice(m_hDevHandle);
    int nRet = MV_CC_DestroyHandle(m_hDevHandle);
    m_hDevHandle = nullptr;
}

void JZCameraHik::startGrabbing()
{
    if (!m_isStartGrabbing)
    {
        MV_CC_StartGrabbing(m_hDevHandle);
        m_isStartGrabbing = true;
        m_thread = QThread::create([this] {
            this->GrabbingThread();
        });
        m_thread->start();
    }
}

void JZCameraHik::GrabbingThread()
{
    while (m_isStartGrabbing)
    {
        MV_FRAME_OUT pFrame = {};
        int nMsec = 1000;
        int nRet = MV_CC_GetImageBuffer(m_hDevHandle, &pFrame, nMsec);
        if (nRet == MV_OK)
        {            
            auto &stImageInfo = pFrame.stFrameInfo;
            int w = pFrame.stFrameInfo.nWidth;
            int h = pFrame.stFrameInfo.nHeight;

            cv::Mat image(h,w, CV_8UC3);
            
            //转换图像格式为BGR8
            MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };
            memset(&stConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
            stConvertParam.nWidth = stImageInfo.nWidth;
            stConvertParam.nHeight = stImageInfo.nHeight;
            stConvertParam.pSrcData = pFrame.pBufAddr;
            stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;
            stConvertParam.enSrcPixelType = stImageInfo.enPixelType;
            stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
            stConvertParam.pDstBuffer = image.data;
            stConvertParam.nDstBufferSize = image.total() * image.elemSize();
            MV_CC_ConvertPixelType(m_hDevHandle, &stConvertParam);            

            MV_CC_FreeImageBuffer(m_hDevHandle, &pFrame);
            emit sigFrameReady(image);
        }
        else
        {

        }
    }
}

void JZCameraHik::start()
{
    if (!m_hDevHandle)
        return;

    startGrabbing();    
    MV_CC_SetEnumValue(m_hDevHandle, "TriggerMode", MV_TRIGGER_MODE_OFF);
}

void JZCameraHik::startOnce()
{
    if (!m_hDevHandle)
        return;

    startGrabbing();    
    MV_CC_SetEnumValue(m_hDevHandle, "TriggerMode", MV_TRIGGER_MODE_ON);
    CommandExecute("TriggerSoftware");
}

void JZCameraHik::stop()
{
    if (m_isStartGrabbing)
    {        
        m_isStartGrabbing = false;
        m_thread->wait();
        m_thread = nullptr;
        MV_CC_StopGrabbing(m_hDevHandle);
    }
}

bool JZCameraHik::CommandExecute(QString command)
{
    int nRet = MV_CC_SetCommandValue(m_hDevHandle, qUtf8Printable(command));
    return (MV_OK == nRet);
}

QString JZCameraHik::config()
{
    if (!m_hDevHandle)
        return QString();

    QJsonObject obj;
    double ExposureTime = GetExposureTime();
    double Gain = GetGain();
    obj["ExposureTime"] = ExposureTime;
    obj["Gain"] = Gain;

    return QString::fromUtf8(QJsonDocument(obj).toJson());
}

bool JZCameraHik::setConfig(const QString &config)
{
    if (!m_hDevHandle)
        return false;

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(config.toUtf8(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError)    
        return false;
    
    QJsonObject obj = doc.object();
    double ExposureTime = obj["ExposureTime"].toDouble();
    double Gain = obj["Gain"].toDouble();
    SetExposureTime(ExposureTime);
    SetGain(Gain);
        
    return true;
}

double JZCameraHik::GetExposureTime()  // ch:设置曝光时间 | en:Set Exposure Time
{
    MVCC_FLOATVALUE stFloatValue = { 0 };

    int nRet = MV_CC_GetFloatValue(m_hDevHandle, "ExposureTime", &stFloatValue);
    return stFloatValue.fCurValue;
}

bool JZCameraHik::SetExposureTime(double time)
{
    MV_CC_SetEnumValue(m_hDevHandle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
    return MV_CC_SetFloatValue(m_hDevHandle, "ExposureTime", (float)time) == MV_OK;
}

double JZCameraHik::GetGain()  // ch:设置增益 | en:Set Gain
{
    MVCC_FLOATVALUE stFloatValue = { 0 };

    int nRet = MV_CC_GetFloatValue(m_hDevHandle, "Gain", &stFloatValue);
    return stFloatValue.fCurValue;
}

bool JZCameraHik::SetGain(double gain)
{
    MV_CC_SetEnumValue(m_hDevHandle, "GainAuto", 0);
    return MV_CC_SetFloatValue(m_hDevHandle, "Gain", (float)gain) == MV_OK;
}