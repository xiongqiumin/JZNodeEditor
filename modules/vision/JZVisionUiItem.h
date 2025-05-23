#ifndef JZ_VISION_UI_FILE_H_
#define JZ_VISION_UI_FILE_H_

#include "JZUiBaseItem.h"
#include "JZNode.h"
#include "JZNodeObject.h"
#include "../JZModuleDefine.h"
#include "JZVisionWindow.h"

enum {
    ProjectItem_visionUi = Module_visionItem,
};

//JZVisionUiItem
class JZVisionUiItem : public JZUiBaseItem
{
public:    
    JZVisionUiItem();
    virtual ~JZVisionUiItem();
    
    virtual JZNodeObjectWidgetDefine define();
    void setConfig(JZVisionWindowConfig config);
    JZVisionWindowConfig config();

protected:
    virtual void saveToStream(QDataStream &s) const override;
    virtual bool loadFromStream(QDataStream &s) override;

    JZVisionWindowConfig m_config;
};



#endif
