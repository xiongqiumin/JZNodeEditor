#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QWidget>
#include <QStackedWidget>
#include "modules/camera/JZCameraNode.h"
#include "modules/model/JZModelWidget.h"
#include "modules/communication/JZCommWidget.h"
#include "jzDatabase/JZDataBase.h"
#include "JZPanelWidget.h"
#include "JZCameraListWidget.h"
#include "JZCameraViewWidget.h"
#include "JZProject.h"
#include "LogWidget.h"
#include "JZProjectTree.h"
#include "JZFlowTree.h"
#include "mainTask.h"
#include "editor/JZVisionEditor.h"
#include "jzWidgets/JZLogWidget.h"
#include "jzWidgets/JZImageLabel.h"
#include "editor/JZVisionEditor.h"
#include "JZNodeTraceView.h"
#include "JZModuleVisionApp.h"

class Setting
{
public:
    Setting();
    void addRecentProject(QString file);

    QStringList recentFile;
};

//MainWindow
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    JZProject *project();

    JZModelManager* modelManager();
    JZCameraManager* cameraManager();
    JZCommManager* commManager();
    
    JZPaddleOCR* getOCR();
    JZBarCode* getBarCode();
    JZQRCode* getQrCode();

    JZCamera *camera(QString name);
    bool openCamera(JZCamera *camera);
    void startCamera(QString name);
    void startCameraOnce(QString name);
    void stopCamera(QString name);

    void imageDebug();    

protected slots:
    void onBtnCamera();
    void onBtnFlow();
    void onBtnModel();
    void onBtnComm();
    void onBtnSetting();
    void onBtnLog();

    void onActionNewProject();
    void onActionOpenProject();
    void onActionCloseProject();
    void onActionRecentProject();

    void onActionUndo();
    void onActionRedo();
    void onActionDel();
    void onActionCut();
    void onActionCopy();
    void onActionPaste();
    void onActionSelectAll();

    void onActionSaveFile();
    void onActionCloseFile();
    void onActionSaveAllFile();
    void onActionCloseAllFile();
    void onActionCloseAllFileExcept();

    void onActionCommTool();
    void onActionProfile();

    void onActionHelp();
    void onActionAbout();
    void onActionSample();

    void onActionBuild();
    void onActionRun();
    void onActionRunOnce();    

    void onEditorClose(int index);
    void onEditorActivity(int index);
    void onNavigate(QUrl url);

    void onCameraConfigChanged();    
    void onModelConfigChanged();
    void onCommConfigChanged();

    void onProjectItemChanged(JZProjectItem* item);
    void onProjectChanged();
    void onProjectTreeAction(int type, QString filepah);

    void onModifyChanged(bool flag);
    void onRedoAvailable(bool flag);
    void onUndoAvailable(bool flag);
    void onTabContextMenu(QPoint pos);

    void onFunctionOpen(QString functionName);
    void onAutoCompiler();
    void onAutoRun();
    void onAutoRunOnce();
    void onAutoRunStop();

    void onBuildStart();
    void onBuildFinish(JZNodeBuildResultPtr result);
    void onBuildLog(int level, QString text);
    void onAutoRunResult(int result);

    void onFrameReady(QString camera,cv::Mat mat);
    void onCameraError(QString camera, QString error);
    void onMainStackedChanged();
    void onRuntimeError(JZNodeRuntimeError error);
    
    void onComToolClose();
    void onFloatWindowDestory();

protected:    
    struct CameraProgram
    {        
        QString function;        
    };

    virtual void customEvent(QEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;

    void initDatabase();
    void loadSetting();
    void saveSetting();    

    void initUi();    
    void initMenuBar(QVBoxLayout *layout);
    QWidget *createTitleBar();
    
    QIcon icon(QString name);
    QIcon menuIcon(const QString &name);    

    void setSliderStyle(QWidget *w);
    void addCameraPage();
    void addFlowPage();
    void addModelPage();
    void addCommPage();
    void addSettingPage();
    void addLogPage();

    void initProject();
    bool openProject(QString filepath);
    bool closeProject();

    void openItem(QString filepath);
    void closeItem(QString filepath);
    void removeItem(QString filepath);

    JZEditor* createEditor(int type);
    bool openEditor(QString filepath);
    void closeEditor(JZEditor* editor);
    JZEditor* editor(QString filepath);
    JZVisionEditor* currentNodeEditor();
    QList<JZVisionEditor*> nodeEditorList();
    JZVisionEditor* nodeEditor(QString filepath);
    void switchEditor(JZEditor* editor);
    bool closeAllEditor(JZEditor* except = nullptr);
    void saveAll();

    void updateActionStatus();
    void updateTabText(int index);
    JZNode *getInitNode(int type);
        
    bool isRun();    
    void stop();

    bool checkBuild();
    bool isCameraFlow();
    JZNodeCameraReadyEvent* currrentCameraNode();
    QString getCameraByProgram(QString function);    

    const CompilerResult* compilerResult(const QString& path);
    void addFlowWindow(QWidget *w);
    
    bool initEngine();
    void releaseEngine();

    JZModelManager* m_modelManager;
    JZCameraManager* m_cameraManager;
    JZCommManager* m_commManager;
    JZQRCode* m_qrCode;
    JZBarCode* m_barCode;
    JZPaddleOCR* m_ocr;

    JZNodeObjectPointer m_app;

    QString m_className;
    QMap<QString, CameraProgram> m_camProgram;

    QStackedWidget *m_stack;    
    JZCameraListWidget *m_cameraList;
    JZCameraViewWidget *m_cameraView;
    JZCommConfigWidget *m_commConfigWidget;
    JZModelConfigWidget *m_modelConfigWidget;    
    JZFlowTree* m_projectTree;    

    Setting m_setting;
    JZDataBase m_db;
    JZDbConfigTable m_dbConfig;
    JZProject m_project;

    QList<QMenu*> m_menuList;
    JZEditor* m_editor;
    QTabWidget* m_editorTab;
    QMap<JZProjectItem*, JZEditor*> m_editors;
    MainTaskManager m_task;
    JZNodeBuildResultPtr m_buildResult;
    JZNodeEngine m_engine;

    JZLogWidget *m_mainLog;
    LogWidget *m_buildLog;

    QAction *m_actionRun;
    QList<QWidget*> m_floatWindow;
    JZNodeTraceView *m_traceView;
    QMap<QString, QString> m_roiFunction;
};
extern MainWindow *g_visionWindow;

#endif // MAINWINDOW_H    