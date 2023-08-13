#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QProcess>
#include "JZProjectTree.h"
#include "JZNodeDebugClient.h"
#include "JZEditor.h"
#include "JZNodeBuilder.h"
#include "JZNodeDebugClient.h"
#include "JZNodeProgram.h"
#include "LogWidget.h"
#include "JZNodeStack.h"
#include "JZNodeBreakPoint.h"

class Setting
{
public:
    Setting();
    void addRecentProject(QString file);

    QStringList recentFile;
};

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
    void onActionRecentProject();

    void onActionNewFile();
    void onActionSaveFile();
    void onActionCloseFile();
    void onActionSaveAllFile();
    void onActionCloseAllFile();

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

    void onModifyChanged(bool flag);
    void onRedoAvailable(bool flag);
    void onUndoAvailable(bool flag);

    void onFileOpened(QString filepath);    
    void onFileClosed(QString filepath);
    void onFileRemoved(QString filepath);
    void onEditorClose(int index);
    void onEditorActivite(int index);
    void onNodeClicked(QString file, int nodeId);
    void onStackChanged(int stack);

    void onRuntimeLog(QString log);    
    void onRuntimeError(JZNodeRuntimeError error);    
    void onRuntimeStatus(int staus);    
    void onRuntimeFinish(int code,QProcess::ExitStatus status);
    void onNetError();

private:
    struct ActionStatus{
        enum {
            ProjectVaild,
            FileOpen,
            FileIsModify,
            FileIsScript,
            ProcessIsEmpty,
            ProcessIsVaild,
            ProcessCanPause,
            ProcessCanResume,  
            Count,
        };
        ActionStatus(QAction *act, QVector<int> flag);

        QVector<int> flags;
        QAction *action;
    };

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

    void loadSetting();
    void saveSetting();
    bool openProject(QString filepath);
    JZEditor *createEditor(int type);
    bool openEditor(QString filepath);
    void closeEditor(JZEditor *editor);
    void updateActionStatus();    

    void initMenu();
    void initUi();        
    void switchEditor(JZEditor *editor);    
    void gotoNode(QString file, int nodeId);
    void setRunning(bool flag);    
    void setRuntimeNode(QString file, int nodeId);

    bool build();
    void start(bool startPause);    
    void saveToFile(QString file,QString text);
    void saveAll();
    bool closeAll();    
    
    JZProject m_project;    

    LogWidget *m_log;
    JZNodeStack *m_stack;
    JZNodeWatch *m_watch;
    JZNodeBreakPoint *m_breakPoint;
    JZProjectTree *m_projectTree;
    QList<QMenu*> m_menuList;
        
    QList<ActionStatus> m_actionStatus;
    QAction *m_actCloseFile, *m_actSaveFile;

    JZEditor *m_editor;  
    QTabWidget *m_editorStack;
    QMap<QString,JZEditor *> m_editors;
    Setting m_setting;

    JZNodeDebugClient m_debuger;
    QProcess m_process;       
    bool m_processVaild;

    QAction *m_actionRun, *m_actionResume;

    JZNodeProgramInfo m_program;
    JZNodeRuntimeInfo m_runtime;
};
#endif // MAINWINDOW_H
