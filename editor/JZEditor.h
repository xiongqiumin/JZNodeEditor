#ifndef JZ_EDITOR_H_
#define JZ_EDITOR_H_

#include <QWidget>
#include "JZProjectItem.h"

class JZEditor : public QWidget
{
    Q_OBJECT

public:
    JZEditor();
    ~JZEditor();
    
    virtual void open(JZProjectItem *item) = 0;
    void close();

    bool isModified();

    void save();
    void saveAs();
    void load();

    void undo();
    void redo();
    void copy();
    void paste();

protected:

};








#endif