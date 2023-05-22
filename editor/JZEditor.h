#ifndef JZ_EDITOR_H_
#define JZ_EDITOR_H_

#include <QWidget>
#include <QMenuBar>
#include "JZProjectItem.h"

class JZEditor : public QWidget
{
    Q_OBJECT

public:
    JZEditor();
    virtual ~JZEditor();
    
    virtual void open(JZProjectItem *item) = 0;
    virtual void close() = 0;
    virtual void save() = 0;

    virtual bool isModified() = 0;
    virtual void updateMenuBar(QMenuBar *menubar);

    virtual void undo();
    virtual void redo();
    virtual void remove();
    virtual void cut();
    virtual void copy();
    virtual void paste();

protected:

};








#endif
