#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QWidget>
#include <QStackedWidget>
#include "modules/communication/JZCommManager.h"
#include "modules/camera/JZCameraManager.h"
#include "modules/camera/JZCameraNode.h"
#include "modules/model/JZModelManager.h"
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

    JZModelManager* modelManager();
    JZCameraManager* cameraManager();
    JZCommManager* commManager();

    JZCamera *camera(QString name);
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

    void onActionHelp();
    void onActionAbout();

    void onActionBuild();
    void onActionRun();
    void onActionRunOnce();
    void onActionStop();

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
    void onAutoRunResult(int result);

    void onFrameReady(QString camera,cv::Mat mat);

    const CompilerResult* compilerResult(const QString& path);

protected:
    struct CameraProgram
    {
        struct ImageResult
        {
            cv::Mat mat;
            QList<JZGraphic> graphList;
        };

        struct Result
        {
            QMap<QString, ImageResult> outputImage;
            QString error;
        };

        QString function;
        QMap<int,Result> resultMap;
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

    bool checkBuild();
    bool isCameraFlow();
    JZNodeCameraReadyEvent* currrentCameraNode();
    void currrentCameraStart(bool is_once);
    void currrentCameraStop();

    JZModelManager* m_modelManager;
    JZCameraManager* m_cameraManager;
    JZCommManager* m_commManager;

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
    JZConfigTable m_config;
    JZProject m_project;

    QList<QMenu*> m_menuList;
    JZEditor* m_editor;
    QTabWidget* m_editorStack;
    QMap<JZProjectItem*, JZEditor*> m_editors;
    MainTaskManager m_task;
    JZNodeBuildResultPtr m_buildResult;
    JZNodeEngine m_engine;

    JZLogWidget *m_mainLog;
    LogWidget *m_buildLog;
};
extern MainWindow *g_visionWindow;

#endif // MAINWINDOW_H    