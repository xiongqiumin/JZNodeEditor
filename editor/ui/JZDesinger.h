#ifndef JZ_DESIGNER_H_
#define JZ_DESIGNER_H_

#include <QtDesigner/QDesignerPropertyEditorInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerObjectInspectorInterface>
#include <QtDesigner/QDesignerWidgetBoxInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerComponents>

class JZDesignerEditor;
class JZDesigner : public QObject
{
    Q_OBJECT

public:
    static JZDesigner *instance();

    JZDesigner();
    ~JZDesigner();

    JZDesignerEditor *editor();
    void closeEditor();

protected:
    JZDesignerEditor *m_editor;
    QDesignerFormEditorInterface *m_core;
};


#endif // !JZDESIGNER_H_
