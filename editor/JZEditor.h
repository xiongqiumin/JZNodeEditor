#ifndef JZ_EDITOR_H_
#define JZ_EDITOR_H_

#include <QWidget>
#include <QMenuBar>
#include "JZProjectItem.h"

class JZProject;
class JZEditor : public QWidget
{
    Q_OBJECT

public:
    JZEditor();
    virtual ~JZEditor();

    void setProject(JZProject *project);

    void setFile(JZProjectItem *file);
    JZProjectItem *file();

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

protected:
    JZProjectItem *m_file;
    JZProject *m_project;
};








#endif
