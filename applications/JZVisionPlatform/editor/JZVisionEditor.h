#ifndef JZ_VISION_EDITOR_H_
#define JZ_VISION_EDITOR_H_

#include <QWidget>
#include <QSplitter>
#include "JZEditor.h"
#include "JZVisionView.h"
#include "JZVisionImage.h"
#include "JZVisionOutput.h"
#include "JZNodeCompiler.h"
#include "JZVisionPanel.h"

class JZVisionEditor : public JZEditor
{
    Q_OBJECT
    
public:
    explicit JZVisionEditor(QWidget *parent = nullptr);
    ~JZVisionEditor();

    void setCompilerResult(const CompilerResult *info);

    virtual void open(JZProjectItem *item) override;
    virtual void close() override;
    virtual void save() override;

    virtual void active() override;
    virtual void inactive‌() override;

    virtual bool isModified() override;
    virtual void navigate(QUrl url) override;

    virtual void undo() override;
    virtual void redo() override;
    virtual void remove() override;
    virtual void cut() override;
    virtual void copy() override;
    virtual void paste() override;
    virtual void selectAll() override;
    
signals:    
    void sigAutoCompiler();
    void sigAutoRunOnce();
    void sigAutoRun();
    void sigAutoRunStop();    

protected slots:
    

protected:
    void init();

    JZVisionPanel *m_nodePanel;
    JZVisionView *m_view;
    JZVisionImage *m_rightImage;
    JZVisionOutput *m_rightOutput;
    QList<QAction*> m_actionList;
};

#endif // !JZ_VISION_EDITOR_H_
