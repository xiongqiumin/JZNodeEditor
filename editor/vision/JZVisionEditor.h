#ifndef JZ_VISION_EDITOR_H_
#define JZ_VISION_EDITOR_H_

#include <QWidget>
#include "JZEditor.h"
#include <QSplitter>
#include "JZVisionView.h"
#include "JZVisionImage.h"
#include "JZVisionOutput.h"
#include "JZVisionPanel.h"

class JZVisionEditor : public JZEditor
{
    Q_OBJECT
    
public:
    explicit JZVisionEditor(QWidget *parent = nullptr);
    ~JZVisionEditor();

signals:

protected slots:

protected:
    void initLayout();

    JZVisionPanel *m_leftPanel;
    JZVisionView *m_centerView;
    JZVisionImage *m_rightImage;
    JZVisionOutput *m_rightOutput;
};

#endif // !JZ_VISION_EDITOR_H_
