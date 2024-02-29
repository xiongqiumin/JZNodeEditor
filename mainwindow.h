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
#include "LogManager.h"
#include "tests/test_server.h"

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

    void onActionNewEvent();
    void onActionNewFunction();
    void onActionNewClass();
    void onActionSaveFile();
    void onActionCloseFile();
    void onActionSaveAllFile();
    void onActionCloseAllFile();
    void onActionCloseAllFileExcept();

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

    void onActionHelp();
    void onActionCheckUpdate();
    void onActionAbout();

    void onModifyChanged(bool flag);
    void onRedoAvailable(bool flag);
    void onUndoAvailable(bool flag);    
    void onProjectTreeAction(int type, QString filepah);

    void onEditorClose(int index);
    void onEditorActivite(int index);
    void onNodeClicked(QString file, int nodeId);
    void onProjectChanged();
    
    void onFunctionOpen(QString filepath);    

    void onLog(LogObjectPtr log);

    void onStackChanged(int stack);
    void onWatchValueChanged(JZNodeParamCoor coor, QVariant value);
    void onWatchNameChanged(JZNodeParamCoor coor);
    void onNodePropChanged(const JZNodeValueChanged &info);

    void onRuntimeLog(QString log);    
    void onRuntimeError(JZNodeRuntimeError error);    
    void onRuntimeStatus(int staus);    
    void onRuntimeFinish(int code,QProcess::ExitStatus status);        
    
    void onTestProcessFinish();
    void onNetError();
    void onTabContextMenu(QPoint pos);

private:
    struct ActionStatus{
        enum {
            ProjectVaild,
            FileOpen,
            FileIsModify,            
            FileIsScript,
            HasModifyFile,
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

    bool closeProject();
    bool openProject(QString filepath);

    void openItem(QString filepath);
    void closeItem(QString filepath);
    void removeItem(QString filepath);

    JZEditor *createEditor(int type);
    bool openEditor(QString filepath);    
    void closeEditor(JZEditor *editor);
    JZEditor *editor(QString filepath);
    void updateActionStatus();    

    void initMenu();    
    void initUi();        
    void switchEditor(JZEditor *editor);    
    void gotoNode(QString file, int nodeId);
    void setRunning(bool flag);    
    void setRuntimeNode(QString file, int nodeId);
    void clearRuntimeNode();
    void updateRuntime(int stack_index, bool isNew);    
    void clearWatchs();
    void setWatchStatus(int status);
    void setWatchRunning(bool flag);
    void updateAutoWatch(int stack_index);

    bool build();
    void start(bool startPause);    
    void saveToFile(QString file,QString text);
    void saveAll();
    bool closeAll(bool except_current = false); 
    void initLocalProcessTest(bool flag);
    QIcon menuIcon(const QString &name);
    
    JZProject m_project;    

    LogWidget *m_log;
    JZNodeStack *m_stack;
    JZNodeWatch *m_watchAuto,*m_watchManual;
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

    bool m_useTestProcess;
    TestServer m_testProcess;

    QList<JZNodeWatch*> m_debugWidgets;
    QAction *m_actionRun, *m_actionResume;
    QList<QAction*> m_debugActions;
    QToolBar *m_toolDebug;

    JZNodeProgram m_program;
    JZNodeRuntimeInfo m_runtime;
};
#endif // MAINWINDOW_H
