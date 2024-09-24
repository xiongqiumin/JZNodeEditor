#include "ImageModule.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"
#include "JZTx.h"

static QImage ImageThreshold(const QImage &image)
{    
    JZTX_FUNCTION
    
    QImage binary = image.convertToFormat(QImage::Format_ARGB32);
    int width = binary.width();
    int height = binary.height();
    int threshold = 128;

    for (int y = 0; y < height; ++y) {
        QRgb *line = (QRgb *)binary.scanLine(y);
        for (int x = 0; x < width; ++x) {
            int gray = qGray(line[x]);
            if (gray < threshold) {
                line[x] = qRgb(0, 0, 0); // 黑色
            } else {
                line[x] = qRgb(255, 255, 255); // 白色
            }
        }
    }

    return binary;
}

ModuleImageSample::ModuleImageSample()
{
    m_name = "imageSample";
    m_functionList << "ImageThreshold";
}

ModuleImageSample::~ModuleImageSample()
{
}

void ModuleImageSample::regist(JZScriptEnvironment *env)
{    
    auto inst = env->functionManager();
    inst->registCFunction("ImageThreshold",false,jzbind::createFuncion(ImageThreshold));
}

void ModuleImageSample::unregist(JZScriptEnvironment *env)
{   
    auto inst = env->functionManager();
    inst->unregistFunction("ImageThreshold");
}