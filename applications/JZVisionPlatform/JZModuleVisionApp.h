#ifndef JZ_MODEL_VISION_APP_H_
#define JZ_MODEL_VISION_APP_H_

#include <QObject>
#include "JZModule.h"
#include "modules/model/JZModelManager.h"
#include "modules/communication/JZCommManager.h"
#include "modules/camera/JZCameraManager.h"
#include "modules/vision/JZVision.h"
#include "modules/model/PaddleOCR/JZPaddleOCR.h"

//JZVisionApplication
class MainWindow;
class JZVisionApplication : public QObject
{
public:
    JZVisionApplication();
    ~JZVisionApplication();

    void setMainWindow(MainWindow* window);

    JZModelManager* modelManager();
    JZCameraManager* cameraManager();
    JZCommManager* commManager();

    JZPaddleOCR* getOCR();
    JZBarCode* getBarCode();
    JZQRCode *getQrCode();

protected:
    MainWindow* m_window;
};

//JZModuleVisionApp
class JZModuleVisionApp : public JZModule
{
public:
    JZModuleVisionApp();
    virtual ~JZModuleVisionApp();

    virtual void regist(JZScriptEnvironment* env) override;
    virtual void unregist(JZScriptEnvironment* env) override;
};


#endif // !JZ_MODEL_VISION_APP_H_