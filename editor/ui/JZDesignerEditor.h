#ifndef JZ_DESIGNER_EDITOR_H_
#define JZ_DESIGNER_EDITOR_H_

#include "JZEditor.h"
#include "JZUiFile.h"
#include "JZDesinger.h"
#include <QStackedWidget>
#include <QtDesigner/QDesignerFormEditorInterface>

class JZDesignerFormWindow;
class JZDesignerEditor : public QWidget
{
    Q_OBJECT
    
public:
    static JZDesignerFormWindow *activeWidgetHost();

    JZDesignerEditor();
    ~JZDesignerEditor();

    void init(QDesignerFormEditorInterface *core);
    QDesignerFormEditorInterface *core();

    JZDesignerFormWindow *open(JZUiFile *file);
    void close(JZDesignerFormWindow *window);

    void showForm(JZDesignerFormWindow *window);
    void hideForm(JZDesignerFormWindow *window);

protected slots:    
    void slotSubWindowActivated(int index);
    void formSizeChanged(int w, int h);

protected:                
    Qt::WindowFlags windowFlag();            

    QList<JZDesignerFormWindow*> m_windows;    
    QWidget *m_empty;
    QStackedWidget *m_view;
    QDesignerFormEditorInterface *m_core;
    static JZDesignerFormWindow *m_active;
};

#endif
