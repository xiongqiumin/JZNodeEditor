#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextEdit>
#include "JZProjectTree.h"
#include "JZEditor.h"
#include "JZNodeBuilder.h"
#include "JZNodeDebugClient.h"
#include "JZNodeProgram.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:    
    void onActionNewProject();
    void onActionOpenProject();
    void onActionCloseProject();
    void onActionSaveProject();
    void onActionSaveAsProject();

    void onActionUndo();
    void onActionRedo();
    void onActionDel();
    void onActionCut();
    void onActionCopy();
    void onActionPaste();

    void onActionBuild();

    void onActionRun();
    void onActionBreakPoint();
    void onActionStepOver();
    void onActionStepIn();
    void onActionStepOut();

    void onFileOpened(QString filepath);    
    void onFileClosed(QString filepath);
    
private:
    void resizeEvent(QResizeEvent *event);
    JZEditor *createEditor(int type);

    void initMenu();
    void initUi();        
    void switchEditor(JZEditor *editor);

    JZNodeBuilder m_builder;
    JZProject m_project;
    JZNodeProgram m_program;

    JZProjectTree *m_projectTree;
    JZEditor *m_editor;  
    QStackedWidget *m_editorStack;
    QTextEdit *m_log;
    JZNodeDebugClient m_debuger;

    QMap<QString,JZEditor *> m_editors;
};
#endif // MAINWINDOW_H
