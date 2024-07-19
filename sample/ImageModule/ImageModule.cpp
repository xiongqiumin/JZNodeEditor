#include "ImageModule.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"

static QImage ImageThreshold(const QImage &image)
{
    return image;
}

//ImageModule
void ImageModule::init()
{
    JZModuleInfo info;
    info.name = "ImageModuleSample";
    info.createFunc = createModule<ImageModule>;

    JZModuleManager::instance()->registModule(info);
}

ImageModule::ImageModule()
{   
    m_info.name = "ImageModuleSample";
    m_info.createFunc = createModule<ImageModule>;
    
    m_functions << "ImageThreshold";
}

ImageModule::~ImageModule()
{
}

void ImageModule::regist() 
{
    auto inst = JZNodeFunctionManager::instance();

    inst->registCFunction("ImageThreshold",false,jzbind::createFuncion(ImageThreshold));
}

void ImageModule::unregist()    
{
    auto inst = JZNodeFunctionManager::instance();
    for(int i = 0; i < m_functions.size(); i++)
        inst->unregistFunction(m_functions[i]);
}