#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QWidget>
#include <QStackedWidget>
#include "modules/communication/JZCommManager.h"
#include "modules/camera/JZCameraManager.h"
#include "modules/model/JZModelManager.h"
#include "JZPanelWidget.h"
#include "JZCameraListWidget.h"
#include "JZCameraViewWidget.h"
#include "JZProject.h"
#include "database.h"
#include "LogWidget.h"
#include "JZProjectTree.h"
#include "JZNodeEditor.h"
#include "JZFlowTree.h"
#include "mainTask.h"
#include "modules/model/JZModelWidget.h"
#include "modules/communication/JZCommWidget.h"
#include "modules/model/JZModelWidget.h"
#include "modules/communication/JZCommWidget.h"

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

    void onActionSaveFile();
    void onActionCloseFile();
    void onActionSaveAllFile();
    void onActionCloseAllFile();
    void onActionCloseAllFileExcept();

    void onActionHelp();

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

    void onFlowRun();
    void onFlowRunOnce();
    void onFlowStop();

protected:
    virtual void customEvent(QEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;

    void loadSetting();
    void saveSetting();

    void initUi();    
    QMenuBar *addMenuBar(QVBoxLayout *layout);
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
    JZNodeEditor* currentNodeEditor();
    QList<JZNodeEditor*> nodeEditorList();
    JZNodeEditor* nodeEditor(QString filepath);
    void switchEditor(JZEditor* editor);
    bool closeAllEditor(JZEditor* except = nullptr);
    void saveAll();

    void updateActionStatus();
    void updateTabText(int index);
    JZNode *getInitNode(int type);

    JZModelManager* m_modelManager;
    JZCameraManager* m_cameraManager;
    JZCommManager* m_commManager;

    QStackedWidget *m_stack;    
    JZCameraListWidget *m_cameraList;
    JZCameraViewWidget *m_cameraView;
    JZCommConfigWidget *m_commConfigWidget;
    JZModelConfigWidget *m_modelConfigWidget;
    JZFlowTree* m_projectTree;
    LogWidget* m_log;

    Setting m_setting;
    DataBaseConfig m_dbConfig;
    JZProject m_project;

    QList<QMenu*> m_menuList;
    JZEditor* m_editor;
    QTabWidget* m_editorStack;
    QMap<JZProjectItem*, JZEditor*> m_editors;
    MainTaskManager m_task;
};

#endif // MAINWINDOW_H    