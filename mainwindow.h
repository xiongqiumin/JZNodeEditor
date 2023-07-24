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
    void onEditorClose(int index);

    void onRuntimeLog(QString log);
    void onRuntimeInfo(JZNodeRuntimeInfo error);
    void onRuntimeError(JZNodeRuntimeError error);
    void onRuntimeStatus(int staus);

    void onDebugFinish(int code,QProcess::ExitStatus status);

private:
    struct ActionStatus{
        enum {
            ProjectVaild,
            FileOpen,
            FileIsModify,
            FileIsScript,
            ProcessIsEmpty,
            ProcessIsVaild,
            ProcessIsRunning,
            ProcessIsPause,  
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
    JZEditor *createEditor(int type);
    void closeEditor(JZEditor *editor);
    void updateActionStatus();    

    void initMenu();
    void initUi();        
    void switchEditor(JZEditor *editor);    
    bool build();
    void start(bool startPause);
    void updateMenuAction();
    void saveToFile(QString file,QString text);
    void saveAll();
    bool closeAll();

    JZNodeBuilder m_builder;
    JZProject m_project;
    JZNodeProgram m_program;

    LogWidget *m_log;
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
};
#endif // MAINWINDOW_H
