#ifndef JZ_CAMERAL_MANAGER_EDITOR_H_
#define JZ_CAMERAL_MANAGER_EDITOR_H_

#include "JZNode.h"
#include "JZNodeGraphItem.h"
#include "JZNodeSettingDialog.h"

class JZCameraInitItem : public JZNodeGraphItem
{
public:
    virtual void updatePin();

protected:    
    void onSetClicked();
    
    BlockPtr m_setting;
};


void JZCameraEditorInit();


#endif // !JZ_CAMERAL_MANAGER_EDITOR_H_
