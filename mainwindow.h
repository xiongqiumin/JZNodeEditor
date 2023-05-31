#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextEdit>
#include <QProcess>
#include "JZProjectTree.h"
#include "JZNodeDebugClient.h"
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
    void onActionSelectAll();

    void onActionBuild();

    void onActionRun();
    void onActionPause();
    void onActionStop();
    void onActionBreakPoint();
    void onActionStepOver();
    void onActionStepIn();
    void onActionStepOut();    

    void onRedoAvailable(bool flag);
    void onUndoAvailable(bool flag);

    void onFileOpened(QString filepath);    
    void onFileClosed(QString filepath);
    void onDebugLog(QString log);
    void onDebugFinish(int code,QProcess::ExitStatus status);

private:
    void resizeEvent(QResizeEvent *event);
    JZEditor *createEditor(int type);

    void initMenu();
    void initUi();        
    void switchEditor(JZEditor *editor);    
    bool build();
    void start(bool startPause);
    void updateMenuAction();

    JZNodeBuilder m_builder;
    JZProject m_project;
    JZNodeProgram m_program;

    JZProjectTree *m_projectTree;
    JZEditor *m_editor;  
    QStackedWidget *m_editorStack;
    QTextEdit *m_log;    
    QMap<QString,JZEditor *> m_editors;
    QList<QMenu*> m_menuList;

    JZNodeDebugClient m_debuger;
    QProcess m_process;

    QAction *m_actRun,*m_actStepOver,*m_actStepIn,*m_actStepOut,*m_actBreakPoint;
};
#endif // MAINWINDOW_H
