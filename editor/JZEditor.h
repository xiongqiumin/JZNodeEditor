#ifndef JZ_EDITOR_H_
#define JZ_EDITOR_H_

#include <QWidget>
#include <QMenuBar>
#include "JZProjectItem.h"

enum{
    Editor_none,
    Editor_script,
    Editor_ui,
    Editor_param,
};

class JZProject;
class JZEditor : public QWidget
{
    Q_OBJECT

public:
    JZEditor();
    virtual ~JZEditor();

    int type();
    void setProject(JZProject *project);

    void setItem(JZProjectItem *item);
    JZProjectItem *item();

    virtual void open(JZProjectItem *item) = 0;
    virtual void close() = 0;
    virtual void save() = 0;

    virtual bool isModified();
    virtual void updateMenuBar(QMenuBar *menubar);

    virtual void undo();
    virtual void redo();
    virtual void remove();
    virtual void cut();
    virtual void copy();
    virtual void paste();
    virtual void selectAll();

signals:
    void redoAvailable(bool available);
    void undoAvailable(bool available);
    void modifyChanged(bool changed);

protected:
    JZProjectItem *m_item;
    JZProject *m_project;
    int m_type;
};








#endif
