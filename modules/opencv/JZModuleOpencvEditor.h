#ifndef JZ_MODULE_OPENCV_EDITOR_H_
#define JZ_MODULE_OPENCV_EDITOR_H_

#include "JZNodeGraphItem.h"
#include "JZOpencvNode.h"
#include "JZBaseDialog.h"
#include "jzWidgets/JZImageLabel.h"
#include "jzWidgets/JZPropertyEditor.h"

//JZOpencvTemplateDialog
class JZOpencvTemplateDialog : public JZBaseDialog
{
    Q_OBJECT

public:
    JZOpencvTemplateDialog(QWidget *parent = nullptr);

    void setConfig(JZTemplateConfig cfg);
    JZTemplateConfig config() const;

protected slots:
    void on_loadImageButton_clicked();
    void on_loadTemplateButton_clicked();
    void on_matchButton_clicked();

private:    
    JZTemplateConfig m_config;
    JZImageLabel *m_label;
    JZImageLabel *m_tempLabel;
    JZPropertyEditor *m_propEditor;

    cv::Mat m_templ;
    cv::Mat m_image;
};

class JZOpencvTemplateItem : public JZNodeGraphItem
{
public:
    JZOpencvTemplateItem(JZNode *node);

protected:
    void onSetClicked();

    BlockPtr m_setting;
};


void JZModuleOpencvEditorInit();

#endif