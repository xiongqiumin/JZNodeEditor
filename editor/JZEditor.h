#ifndef JZ_EDITOR_H_
#define JZ_EDITOR_H_

#include <QWidget>
#include <QMenuBar>
#include "JZProjectItem.h"
#include "JZScriptItem.h"
#include "JZEditorGlobal.h"

enum {
    Menu_File,
    Menu_Edit,
    Menu_View,
    Menu_Compiler,
    Menu_Debug,
    Menu_Help,
};

class JZProject;
class JZEditor : public QWidget
{
    Q_OBJECT

public:
    JZEditor(QWidget *parent = nullptr);
    virtual ~JZEditor();

    int type();
    
    void setProject(JZProject *project);
    JZProject *project();

    void setItem(JZProjectItem *item);
    JZProjectItem *item();

    virtual void open(JZProjectItem *item) = 0;
    virtual void close() = 0;
    virtual void save() = 0;
    virtual void active();
    virtual void inactive‌();
    virtual void navigate(QUrl url);
    virtual bool isModified();   

public slots:
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
    QMenu *menu(int type);

    JZProjectItem *m_item;
    JZProject *m_project;
    int m_type;    
};
typedef JZEditor *(*CreateJZEditorFunc)();

template <class T>
JZEditor *CreateEditor() { return new T(); }

//JZEditorManager
class JZEditorManager
{
public:
    static JZEditorManager *instance();

    void registEditor(int project_item_type, CreateJZEditorFunc func);
    JZEditor *createEditor(int project_item_type);

protected:
    JZEditorManager();
    ~JZEditorManager();

    QMap<int, CreateJZEditorFunc> m_editorFunc;
};

#endif
