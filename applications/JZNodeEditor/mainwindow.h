#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QProcess>
#include "JZProjectTree.h"
#include "JZEditor.h"
#include "JZNodeBuilder.h"
#include "JZNodeDebugClient.h"
#include "JZNodeProgram.h"
#include "LogWidget.h"
#include "JZNodeStack.h"
#include "JZNodeBreakPointWidget.h"
#include "LogManager.h"
#include "JZNodeEditor.h"
#include "mainTask.h"
#include "database.h"
#include "JZDebugSettingDialog.h"
#include "remote/JZRemoteClient.h"

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
        
    JZProject *project();    

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

    void onActionProjectProp();

    void onActionBuild();
    void onActionExportExe();
    void onActionExportCpp();

    void onActionRun();
    void onActionDetach();
    void onActionPause();
    void onActionResume();
    void onActionStop();

    void onActionBreakPoint();
    void onActionStepOver();
    void onActionStepIn();
    void onActionStepOut();
    void onActionDebugSetting();

    void onActionModbus();

    void onActionHelp();
    void onActionCheckUpdate();
    void onActionAbout();

    void onModifyChanged(bool flag);
    void onRedoAvailable(bool flag);
    void onUndoAvailable(bool flag);    
    void onProjectTreeAction(int type, QString filepah);

    void onEditorClose(int index);
    void onEditorActivite(int index);
    void onNavigate(QUrl url);
    void onProjectItemChanged(JZProjectItem *item);
    void onProjectChanged();
    
    void onFunctionOpen(QString filepath);    
    void onAutoCompiler();
    void onAutoRunOnce();
    void onAutoRun();
    void onAutoRunStop();

    void onStackChanged(int stack);
    void onSetWatch(JZNodeIRParam coor, QString value);
    void onGetWatch(JZNodeIRParam coor);
    void onRuntimeWatch(const JZNodeRuntimeWatchResult &info);
    void onWatchNotify();
    void onEditorValueChanged(int id,QString value);

    void onRuntimeLog(QString log);    
    void onRuntimeError(JZNodeRuntimeError error);    
    void onRuntimeStatus(int staus);    
    void onRuntimeFinish(int code,QProcess::ExitStatus status);                    

    void onNetError();
    void onTabContextMenu(QPoint pos);
        
    void onBuildStart();    
    void onBuildFinish(JZNodeBuildResultPtr result);
    void onTaskRunning();
    void onAutoRunResult(int result);

    void onBreakPointClicked(QString file, int id);
    void onBreakPointChanged(BreakPointChange reason,QString file, int id);

    void onModbusSimulatorClose();
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
            ProcessCanStartResume,
            Count,
        };
        ActionStatus(QAction *act, QVector<int> flag);

        QVector<int> flags; //同时满足才enable
        QAction *action;
    };    

    virtual void customEvent(QEvent *event) override;
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
    JZNodeEditor *currentNodeEditor();
    QList<JZNodeEditor*> nodeEditorList();
    JZNodeEditor *nodeEditor(QString filepath);
    void updateActionStatus();    

    void initMenu();    
    void initUi();        
    void switchEditor(JZEditor *editor);    
    void gotoNode(QString file, int nodeId);
    void setRunningMode(ProcessStatus flag);
    void setRuntimeNode(QString file, int nodeId);
    void clearRuntimeNode();
    void updateRuntime(int stack_index, bool isNew);
    void setRuntimeValue(QString file,int node_id,int pin_id,const JZNodeDebugParamValue &value);    
    JZNodeRuntimeInfo::Stack *currentStack();
        
    void setWatchStatus(ProcessStatus status);
        
    void startProgram();    
    void stopProgram();
    void startUnitTest(QString testItemPath);
    void stopUnitTest();
    void saveToFile(QString file,QString text);
    void saveAll();
    bool closeAllEditor(JZEditor *except = nullptr); 
    QIcon menuIcon(const QString &name);
    void showTopLevel();
    void updateTabText(int index);  
    
    const CompilerResult *compilerResult(const QString &path);
    void updateAutoRunDepend();

    JZProject m_project;    

    LogWidget *m_log;
    JZNodeStack *m_stack;
    JZNodeWatch *m_watch;
    JZNodeBreakPointWidget *m_breakPoint;
    JZProjectTree *m_projectTree;
    QList<QMenu*> m_menuList;
        
    QList<ActionStatus> m_actionStatus;
    QAction *m_actCloseFile, *m_actSaveFile;

    JZEditor *m_editor;  
    QTabWidget *m_editorStack;
    QMap<JZProjectItem*,JZEditor *> m_editors;
    Setting m_setting;

    JZNodeDebugClient m_debuger;
    QProcess m_process;           
    ProcessStatus m_processMode;

    JZRemoteClient m_remote;
    
    QAction *m_actionRun, *m_actionResume;
    QList<QAction*> m_debugActions;
    QToolBar *m_toolDebug;        

    JZNodeProgram m_program;
    JZNodeRuntimeInfo m_runtime;    
    MainTaskManager m_task;
    JZNodeBuildResultPtr m_buildResult;
    JZDebugSetting m_debugSettting;

    DataBaseConfig m_config;

    QList<QWidget*> m_floatWidgets;
};
extern MainWindow *g_mainWindow;

#endif // MAINWINDOW_H
