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
    void onActionNewFile();
    void onActionSaveFile();
    void onActionCloseFile();

    void onActionUndo();
    void onActionRedo();
    void onActionDel();
    void onActionCut();
    void onActionCopy();
    void onActionPaste();
    void onActionSelectAll();

    void onActionBuild();

    void onActionRun();
    void onActionDetach();
    void onActionPause();
    void onActionResume();
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
    void onRuntimeError(JZNodeRuntimeError error);
    void onDebugFinish(int code,QProcess::ExitStatus status);

private:
    void resizeEvent(QResizeEvent *event);
    JZEditor *createEditor(int type);
    void closeEditor(JZEditor *editor);

    void initMenu();
    void initUi();        
    void switchEditor(JZEditor *editor);    
    bool build();
    void start(bool startPause);
    void updateMenuAction();
    void saveToFile(QString file,QString text);
    void saveAll();
    void closeAll();

    JZNodeBuilder m_builder;
    JZProject m_project;
    JZNodeProgram m_program;

    QTextEdit *m_log;
    JZProjectTree *m_projectTree;
    QList<QMenu*> m_menuList;
    QAction *m_actRun,*m_actDetach,*m_actPause,*m_actResume,*m_actStop,*m_actStepOver,*m_actStepIn,*m_actStepOut,*m_actBreakPoint;

    JZEditor *m_editor;  
    QStackedWidget *m_editorStack;       
    QMap<QString,JZEditor *> m_editors;    

    JZNodeDebugClient m_debuger;
    QProcess m_process;   
};
#endif // MAINWINDOW_H
