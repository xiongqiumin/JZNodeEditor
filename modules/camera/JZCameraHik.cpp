#include <QUrl>
#include <QThread>
#include <QJsonParseError>
#include <QJsonObject>
#include <QLibrary>
#include <QDebug>
#include "JZCameraHik.h"

//JZCameraHikConfig
JZCameraHikConfig::JZCameraHikConfig()
{
    type = Camera_Hik;
    triggerMode = TRIGGER_MODE_OFF;
    triggerSource = TRIGGER_SOURCE_SOFTWARE;   //触发

    gainMode = GAIN_MODE_OFF;
    gain = 0;

    exposureMode = EXPOSURE_AUTO_MODE_OFF;
    exposureTime = 20000;
}

void JZCameraHikConfig::saveToStream(QDataStream& s) const
{
    JZCameraConfig::saveToStream(s);
    s << path;
    s << triggerSource;
    s << triggerMode;

    s << gainMode;
    s << gain;

    s << exposureMode;
    s << exposureTime;
}

void JZCameraHikConfig::loadFromStream(QDataStream& s)
{
    JZCameraConfig::loadFromStream(s);
    s >> path;
    s >> triggerSource;
    s >> triggerMode;

    s >> gainMode;
    s >> gain;

    s >> exposureMode;
    s >> exposureTime;
}

#if 0
#include "E:\libs\MVS\Development\Includes\MvCameraControl.h"

// 函数指针类型定义
typedef int(__stdcall *MV_CC_CreateHandleFunc)(void** handle, const MV_CC_DEVICE_INFO* pstDevInfo);
typedef int(__stdcall *MV_CC_DestroyHandleFunc)(void* handle);
typedef int(__stdcall *MV_CC_EnumDevicesFunc)(unsigned int nTLayerType, MV_CC_DEVICE_INFO_LIST* pstDevList);
typedef int(__stdcall *MV_CC_OpenDeviceFunc)(void* handle, unsigned int nAccessMode, unsigned short nSwitchoverKey);
typedef int(__stdcall *MV_CC_CloseDeviceFunc)(void* handle);
typedef int(__stdcall *MV_CC_StartGrabbingFunc)(void* handle);
typedef int(__stdcall *MV_CC_StopGrabbingFunc)(void* handle);
typedef int(__stdcall *MV_CC_GetImageBufferFunc)(void* handle, MV_FRAME_OUT* pstFrame, unsigned int nMsec);
typedef int(__stdcall *MV_CC_FreeImageBufferFunc)(void* handle, MV_FRAME_OUT* pstFrame);
typedef int(__stdcall *MV_CC_SetEnumValueFunc)(void* handle, const char* strKey, unsigned int nValue);
typedef int(__stdcall *MV_CC_SetFloatValueFunc)(void* handle, const char* strKey, float fValue);
typedef int(__stdcall *MV_CC_SetCommandValueFunc)(void* handle, const char* strKey);
typedef int(__stdcall *MV_CC_ConvertPixelTypeFunc)(void* handle, MV_CC_PIXEL_CONVERT_PARAM* pstCvtParam);

class JZCameraHikApi
{
public:
    static JZCameraHikApi *instance();

    bool updateDeviceList();
    
    MV_CC_CreateHandleFunc CreateHandle;
    MV_CC_DestroyHandleFunc DestroyHandle;
    MV_CC_EnumDevicesFunc EnumDevices;
    MV_CC_OpenDeviceFunc OpenDevice;
    MV_CC_CloseDeviceFunc CloseDevice;
    MV_CC_StartGrabbingFunc StartGrabbing;
    MV_CC_StopGrabbingFunc StopGrabbing;
    MV_CC_GetImageBufferFunc GetImageBuffer;
    MV_CC_FreeImageBufferFunc FreeImageBuffer;
    MV_CC_SetEnumValueFunc SetEnumValue;
    MV_CC_SetFloatValueFunc SetFloatValue;
    MV_CC_SetCommandValueFunc SetCommandValue;
    MV_CC_ConvertPixelTypeFunc ConvertPixelType;

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
    // 加载DLL
    QLibrary mvLibrary("MvCameraControl.dll");
    if (!mvLibrary.load()) {
        qCritical() << "Failed to load library:" << mvLibrary.errorString();
        return;
    }

    // 解析函数地址
    CreateHandle = (MV_CC_CreateHandleFunc)mvLibrary.resolve("MV_CC_CreateHandle");
    DestroyHandle = (MV_CC_DestroyHandleFunc)mvLibrary.resolve("MV_CC_DestroyHandle");
    EnumDevices = (MV_CC_EnumDevicesFunc)mvLibrary.resolve("MV_CC_EnumDevices");
    OpenDevice = (MV_CC_OpenDeviceFunc)mvLibrary.resolve("MV_CC_OpenDevice");
    CloseDevice = (MV_CC_CloseDeviceFunc)mvLibrary.resolve("MV_CC_CloseDevice");
    StartGrabbing = (MV_CC_StartGrabbingFunc)mvLibrary.resolve("MV_CC_StartGrabbing");
    StopGrabbing = (MV_CC_StopGrabbingFunc)mvLibrary.resolve("MV_CC_StopGrabbing");
    GetImageBuffer = (MV_CC_GetImageBufferFunc)mvLibrary.resolve("MV_CC_GetImageBuffer");
    FreeImageBuffer = (MV_CC_FreeImageBufferFunc)mvLibrary.resolve("MV_CC_FreeImageBuffer");
    SetEnumValue = (MV_CC_SetEnumValueFunc)mvLibrary.resolve("MV_CC_SetEnumValue");
    SetFloatValue = (MV_CC_SetFloatValueFunc)mvLibrary.resolve("MV_CC_SetFloatValue");
    SetCommandValue = (MV_CC_SetCommandValueFunc)mvLibrary.resolve("MV_CC_SetCommandValue");
    ConvertPixelType = (MV_CC_ConvertPixelTypeFunc)mvLibrary.resolve("MV_CC_ConvertPixelType");

    if (!CreateHandle || !DestroyHandle || !EnumDevices || !OpenDevice ||
        !CloseDevice || !StartGrabbing || !StopGrabbing || !GetImageBuffer || !FreeImageBuffer ||
        !SetEnumValue || !SetFloatValue || !SetCommandValue || !ConvertPixelType) {        
        mvLibrary.unload();
        qFatal("Failed to load MvCameraControl.dll");
    }
}

JZCameraHikApi::~JZCameraHikApi()
{
}

bool JZCameraHikApi::updateDeviceList()
{
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // en:Enumerate all devices within subnet
    int nRet = EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_GIGE_DEVICE | MV_GENTL_CAMERALINK_DEVICE |
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

JZCameraType JZCameraHik::type()
{
    return Camera_Hik;
}

QString JZCameraHik::errorString(int errorCode)
{
    switch (errorCode) {
    case MV_E_HANDLE:
        return "错误或无效的句柄";
    case MV_E_SUPPORT:
        return "不支持的功能";
    case MV_E_BUFOVER:
        return "缓存已满";
    case MV_E_CALLORDER:
        return "函数调用顺序错误";
    case MV_E_PARAMETER:
        return "错误的参数";
    case MV_E_RESOURCE:
        return "资源申请失败";
    case MV_E_NODATA:
        return "无数据";
    case MV_E_PRECONDITION:
        return "前置条件有误，或运行环境已发生变化";
    case MV_E_VERSION:
        return "版本不匹配";
    case MV_E_NOENOUGH_BUF:
        return "传入的内存空间不足";
    case MV_E_ABNORMAL_IMAGE:
        return "异常图像，可能是丢包导致图像不完整";
    case MV_E_LOAD_LIBRARY:
        return "动态导入DLL失败";
    case MV_E_NOOUTBUF:
        return "没有可输出的缓存";
    case MV_E_ENCRYPT:
        return "加密错误";
    case MV_E_OPENFILE:
        return "打开文件出现错误";
    case MV_E_BUF_IN_USE:
        return "缓存地址已使用";
    case MV_E_BUF_INVALID:
        return "无效的缓存地址";
    case MV_E_NOALIGN_BUF:
        return "缓存对齐异常";
    case MV_E_NOENOUGH_BUF_NUM:
        return "缓存个数不足";
    case MV_E_PORT_IN_USE:
        return "串口被占用";
    case MV_E_IMAGE_DECODEC:
        return "解码错误(SDK校验图像异常)";
    case MV_E_UINT32_LIMIT:
        return "图像大小超过unsigned int返回，接口不支持";
    case MV_E_UNKNOW:
        return "未知的错误";
    case MV_E_GC_GENERIC:
        return "通用错误";
    case MV_E_GC_ARGUMENT:
        return "参数非法";
    case MV_E_GC_RANGE:
        return "值超出范围";
    case MV_E_GC_PROPERTY:
        return "属性";
    case MV_E_GC_RUNTIME:
        return "运行环境有问题";
    case MV_E_GC_LOGICAL:
        return "逻辑错误";
    case MV_E_GC_ACCESS:
        return "节点访问条件有误";
    case MV_E_GC_TIMEOUT:
        return "超时";
    case MV_E_GC_DYNAMICCAST:
        return "转换异常";
    case MV_E_GC_UNKNOW:
        return "GenICam未知错误";
    case MV_E_NOT_IMPLEMENTED:
        return "命令不被设备支持";
    case MV_E_INVALID_ADDRESS:
        return "访问的目标地址不存在";
    case MV_E_WRITE_PROTECT:
        return "目标地址不可写";
    case MV_E_ACCESS_DENIED:
        return "设备无访问权限";
    case MV_E_BUSY:
        return "设备忙，或网络断开";
    case MV_E_PACKET:
        return "网络包数据错误";
    case MV_E_NETER:
        return "网络相关错误";
    case MV_E_SUPPORT_MODIFY_DEVICE_IP:
        return "在固定IP模式下不支持修改设备IP模式";
    case MV_E_KEY_VERIFICATION:
        return "秘钥校验错误";
    case MV_E_IP_CONFLICT:
        return "设备IP冲突";
    case MV_E_USB_READ:
        return "读usb出错";
    case MV_E_USB_WRITE:
        return "写usb出错";
    case MV_E_USB_DEVICE:
        return "设备异常";
    case MV_E_USB_GENICAM:
        return "GenICam相关错误";
    case MV_E_USB_BANDWIDTH:
        return "带宽不足";
    case MV_E_USB_DRIVER:
        return "驱动不匹配或者未装驱动";
    case MV_E_USB_UNKNOW:
        return "USB未知的错误";
    case MV_E_UPG_FILE_MISMATCH:
        return "升级固件不匹配";
    case MV_E_UPG_LANGUSGE_MISMATCH:
        return "升级固件语言不匹配";
    case MV_E_UPG_CONFLICT:
        return "升级冲突（设备已经在升级了再次请求升级即返回此错误）";
    case MV_E_UPG_INNER_ERR:
        return "升级时设备内部出现错误";
    case MV_E_UPG_UNKNOW:
        return "升级时未知错误";
    default:
        return "未知错误码";
    }
}

bool JZCameraHik::isOpen()
{
    return m_hDevHandle != nullptr;
}

bool JZCameraHik::open()
{
    auto cfg = dynamic_cast<JZCameraHikConfig*>(m_config.data());
    g_api->updateDeviceList();
    
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
            
            if (QUrl(cfg->path) == QUrl(device_ip))
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

    int nRet = g_api->CreateHandle(&m_hDevHandle, pstDeviceInfo);
    if (MV_OK != nRet)
    {
        return nRet;
    }
    
    nRet = g_api->OpenDevice(m_hDevHandle, MV_ACCESS_Exclusive,0);
    if (MV_OK != nRet)
    {
        g_api->DestroyHandle(m_hDevHandle);
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

    g_api->CloseDevice(m_hDevHandle);
    int nRet = g_api->DestroyHandle(m_hDevHandle);
    m_hDevHandle = nullptr;
}

void JZCameraHik::startGrabbing()
{
    if (!m_isStartGrabbing)
    {
        g_api->StartGrabbing(m_hDevHandle);
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
        int nRet = g_api->GetImageBuffer(m_hDevHandle, &pFrame, nMsec);
        if (nRet == MV_OK)
        {            
            auto &stImageInfo = pFrame.stFrameInfo;
            int w = pFrame.stFrameInfo.nWidth;
            int h = pFrame.stFrameInfo.nHeight;

            cv::Mat image(h,w, CV_8UC3);
                       
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
            g_api->ConvertPixelType(m_hDevHandle, &stConvertParam);                        

            g_api->FreeImageBuffer(m_hDevHandle, &pFrame);
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
    g_api->SetEnumValue(m_hDevHandle, "TriggerMode", MV_TRIGGER_MODE_OFF);
}

void JZCameraHik::startOnce()
{
    if (!m_hDevHandle)
        return;

    startGrabbing();    
    int nRet = g_api->SetEnumValue(m_hDevHandle, "TriggerMode", MV_TRIGGER_MODE_ON);
    if (nRet != MV_OK)
    {
        qDebug() << errorString(nRet);
    }
    CommandExecute("TriggerSoftware");
}

void JZCameraHik::stop()
{
    if (m_isStartGrabbing)
    {        
        m_isStartGrabbing = false;
        m_thread->wait();
        m_thread = nullptr;
        g_api->StopGrabbing(m_hDevHandle);
    }
}

bool JZCameraHik::setConfig(JZCameraConfigEnum config)
{
    m_config = config;
    if (!isOpen())
        return true;

    auto cfg = dynamic_cast<JZCameraHikConfig*>(m_config.data());

    g_api->SetEnumValue(m_hDevHandle, "TriggerSource", cfg->triggerSource);
    g_api->SetEnumValue(m_hDevHandle, "TriggerMode", cfg->triggerMode);

    if (cfg->gainMode == JZCameraHikConfig::GAIN_MODE_OFF)
    {
        g_api->SetEnumValue(m_hDevHandle, "GainAuto", 0);
        g_api->SetFloatValue(m_hDevHandle, "Gain", (float)cfg->gain);
    }
    else if (cfg->gainMode == JZCameraHikConfig::GAIN_MODE_ONCE)
    {
        g_api->SetEnumValue(m_hDevHandle, "GainAuto", MV_GAIN_MODE_ONCE);
    }
    else if (cfg->gainMode == JZCameraHikConfig::GAIN_MODE_CONTINUOUS)
    {
        g_api->SetEnumValue(m_hDevHandle, "GainAuto", MV_GAIN_MODE_CONTINUOUS);
    }

    //exposure
    if (cfg->exposureMode == JZCameraHikConfig::EXPOSURE_AUTO_MODE_OFF)
    {
        g_api->SetEnumValue(m_hDevHandle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
        g_api->SetFloatValue(m_hDevHandle, "ExposureTime", (float)cfg->exposureTime);
    }
    else if (cfg->exposureMode == JZCameraHikConfig::EXPOSURE_AUTO_MODE_ONCE)
    {
        g_api->SetEnumValue(m_hDevHandle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_ONCE);
    }
    else
    {
        g_api->SetEnumValue(m_hDevHandle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_CONTINUOUS);
    }

    return true;
}

bool JZCameraHik::CommandExecute(QString command)
{
    int nRet = g_api->SetCommandValue(m_hDevHandle, qUtf8Printable(command));
    if (nRet != MV_OK)
    {
        qDebug() << errorString(nRet);
    }
    return (MV_OK == nRet);
}
#else

//JZCameraHik
JZCameraHik::JZCameraHik()
{
}

JZCameraHik::~JZCameraHik()
{
    close();
}

JZCameraType JZCameraHik::type()
{
    return Camera_Hik;
}

bool JZCameraHik::setConfig(JZCameraConfigEnum config)
{
    return true;
}

bool JZCameraHik::isOpen()
{
    return m_hDevHandle != nullptr;
}

bool JZCameraHik::open()
{
    return true;
}

void JZCameraHik::close()
{
}

void JZCameraHik::startGrabbing()
{
}

void JZCameraHik::GrabbingThread()
{
}

void JZCameraHik::start()
{
}

void JZCameraHik::startOnce()
{
}

void JZCameraHik::stop()
{
}

bool JZCameraHik::CommandExecute(QString command)
{
    return false;
}

#endif