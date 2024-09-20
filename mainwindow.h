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
#include "JZNodeAutoRunThread.h"
#include "JZNodeBuildThread.h"
#include "JZNodeEditor.h"
#include "editor/tools/JZModbusSimulator.h"

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

    const CompilerResult *compilerResult(const QString &path);
    JZNodeProgram *program();
    JZNodeRuntimeInfo *runtime();
    int stackIndex();

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
    void onActionExport();

    void onActionRun();
    void onActionDetach();
    void onActionPause();
    void onActionResume();
    void onActionStop();

    void onActionBreakPoint();
    void onActionStepOver();
    void onActionStepIn();
    void onActionStepOut();    

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
    void onAutoRun();

    void onLog(LogObjectPtr log);

    void onStackChanged(int stack);
    void onWatchValueChanged(JZNodeIRParam coor, QString value);
    void onWatchNameChanged(JZNodeIRParam coor);
    void onRuntimeWatch(const JZNodeRuntimeWatch &info);
    void onWatchNotify();
    void onEditorValueChanged(int id,QString value);

    void onRuntimeLog(QString log);    
    void onRuntimeError(JZNodeRuntimeError error);    
    void onRuntimeStatus(int staus);    
    void onRuntimeFinish(int code,QProcess::ExitStatus status);        
        
    void onNetError();
    void onTabContextMenu(QPoint pos);

    void onAutoCompilerTimer();
    void onBuildFinish(bool flag);
    void onAutoRunResult(UnitTestResultPtr result);

    void onBreakPointClicked(QString file, int id);
    void onBreakPointChanged(BreakPointChange reason,QString file, int id);

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

    struct BuildInfo
    {
        BuildInfo();

        bool isUnitTest();
        void clear();
        void clearTask();

        bool success;
        qint64 changeTimestamp;
        qint64 buildTimestamp;
        qint64 saveTimestamp;
        QString runItemPath;
        bool save;
        bool start;
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
    void clearWatchs();
    void setWatchStatus(ProcessStatus status);
    void updateAutoWatch(int stack_index);

    void build();
    void saveProgram();
    void startProgram();
    void startUnitTest(QString testItemPath);
    void saveToFile(QString file,QString text);
    void saveAll();
    bool closeAllEditor(JZEditor *except = nullptr);
    void resetEditor(JZEditor *editor);    
    QIcon menuIcon(const QString &name);
    void showTopLevel();
    void updateTabText(int index);
    
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
    QMap<JZProjectItem*,JZEditor *> m_editors;
    Setting m_setting;

    JZNodeDebugClient m_debuger;
    QProcess m_process;           
    ProcessStatus m_processMode;    

    QList<JZNodeWatch*> m_debugWidgets;
    QAction *m_actionRun, *m_actionResume;
    QList<QAction*> m_debugActions;
    QToolBar *m_toolDebug;    
    JZModbusSimulator *m_simulator;

    QTimer *m_compilerTimer;
    BuildInfo m_buildInfo;
    JZNodeAutoRunThread m_runThread;
    JZNodeBuildThread m_buildThread;

    JZNodeProgram m_program;
    JZNodeRuntimeInfo m_runtime;
};
#endif // MAINWINDOW_H
