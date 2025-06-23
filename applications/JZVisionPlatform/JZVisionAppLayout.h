#ifndef JZ_VISION_APP_LAYOUT_H_
#define JZ_VISION_APP_LAYOUT_H_

#include "JZProject.h"
#include "JZScriptItem.h"
#include "editor/JZVisionView.h"

class JZVisionAppLayout
{
public:
    JZVisionAppLayout();
    
    void layout(JZProject *project);
    void layout(JZScriptItem *item);

    
};









#endif // ! JZ_VISION_APP_LAYOUT_H_
