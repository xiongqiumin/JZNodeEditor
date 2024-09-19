#include "ImageModule.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"

static QImage ImageThreshold(const QImage &image)
{
    return image;
}

ModuleImageSample::ModuleImageSample()
{
    m_name = "imageSample";
    m_functionList << "ImageThreshold";
}

ModuleImageSample::~ModuleImageSample()
{
}

void ModuleImageSample::regist() 
{
    auto inst = JZNodeFunctionManager::instance();
    inst->registCFunction("ImageThreshold",false,jzbind::createFuncion(ImageThreshold));
}

void ModuleImageSample::unregist()    
{
    auto inst = JZNodeFunctionManager::instance();
    inst->unregistFunction("ImageThreshold");
}