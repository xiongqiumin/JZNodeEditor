#ifndef JZ_VISION_MANAGER_EDITOR_H_
#define JZ_VISION_MANAGER_EDITOR_H_

#include "JZNodeSettingDialog.h"
#include "JZNodeGraphItem.h"
#include "JZVision.h"
#include "jzWidgets/JZImageLabel.h"

//JZVisionTemplateDialog
class JZVisionTemplateDialog : public JZBaseDialog
{
    Q_OBJECT

public:
    JZVisionTemplateDialog(QWidget *parent = nullptr);

    void setConfig(JZTemplateConfig cfg);
    JZTemplateConfig config() const;

protected slots:
    void on_loadImageButton_clicked();
    void on_loadTemplateButton_clicked();
    void on_matchButton_clicked();

private:    
    void loadTemplate(QString path);

    JZTemplateConfig m_config;
    JZTemplateMatch m_temp;
    JZImageLabel *m_label;
    JZImageLabel *m_tempLabel;
    JZPropertyEditor *m_propEditor;    

    cv::Mat m_templ;
    cv::Mat m_image;
};

class JZVisionTemplateItem : public JZNodeGraphItem
{
public:
    JZVisionTemplateItem(JZNode *node);

protected:
    void onSetClicked();

    BlockPtr m_setting;
};

void JZModuleVisionEditorInit();

#endif // !JZ_CAMERAL_MANAGER_EDITOR_H_
