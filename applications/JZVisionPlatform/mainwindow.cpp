#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QFrame>
#include <QMenuBar>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QApplication>
#include "mainwindow.h"
#include "JZTitleWidget.h"
#include "JZEditorUtils.h"
#include "JZNewProjectDialog.h"
#include "modules/model/JZModelWidget.h"
#include "modules/communication/JZCommWidget.h"
#include "jzWidgets/JZLogWidget.h"
#include "modules/camera/JZCameraNode.h"
#include "modules/model/JZModelNode.h"
#include "modules/communication/JZCommNode.h"
#include "JZModule.h"

//Setting
Setting::Setting()
{

}

void Setting::addRecentProject(QString file)
{
    recentFile.removeAll(file);
    recentFile.insert(0, file);
    if (recentFile.size() > 10)
        recentFile.pop_back();
}

QDataStream& operator<<(QDataStream& s, const Setting& param)
{
    s << param.recentFile;
    return s;
}

QDataStream& operator >> (QDataStream& s, Setting& param)
{
    s >> param.recentFile;
    return s;
}

//MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{            
    setMinimumSize(800, 600);

    JZModuleManager::instance()->addModule();

    m_modelManager = new JZModelManager(this);
    m_cameraManager = new JZCameraManager(this);
    m_commManager = new JZCommManager(this);

    m_list = new JZCameraListWidget();
    m_view = new JZCameraViewWidget();
    m_projectTree = nullptr;

    connect(m_list, &JZCameraListWidget::sigCameraChanged, this, &MainWindow::onCameraConfigChanged);
    
    m_list->setCameraManager(m_cameraManager);
    m_list->setViewWidget(m_view);    

    initUi();

    m_dbConfig.init(qApp->applicationDirPath() + "/config.db");
    loadSetting();
}

MainWindow::~MainWindow()
{
    saveSetting();
}

void MainWindow::loadSetting()
{
    m_setting = m_dbConfig.getConfig<Setting>("setting");
    if (!m_setting.recentFile.isEmpty())
        openProject(m_setting.recentFile[0]);
}

void MainWindow::saveSetting()
{
    m_dbConfig.setConfig<Setting>("setting", m_setting);
}

void MainWindow::setSliderStyle(QWidget* w)
{
    w->setStyleSheet(
        R"(QSplitter::handle
        {
            background-color: rgb(41,57,85);
            width: 6px;
            height: 6px;
        })"
    );
}

void MainWindow::customEvent(QEvent* event)
{
    if (event->type() == JZLogEvent::EventType)
    {
        auto log_event = dynamic_cast<JZLogEvent*>(event);
        auto log = log_event->log;
        //m_log->addLog(log->module, log->message);
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!closeProject())
    {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

QIcon MainWindow::icon(QString name)
{
    QString icon_path = ":/JZMonitorSystem/Resources/icons/" + name;
    return QIcon(icon_path);
}

void MainWindow::initUi()
{
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);        
    
    //title
    auto title = createTitleBar();
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    centralWidget->setLayout(mainLayout);
    mainLayout->addWidget(title);
    mainLayout->setSpacing(1);

    QMenuBar *bar = createMenuBar();
    mainLayout->addWidget(bar);

    //bottom
    QHBoxLayout *bottom_layout = new QHBoxLayout();        
    bottom_layout->setContentsMargins(6, 6, 6, 6);
    bottom_layout->setSpacing(0);

    QWidget *buttonWidget = new QWidget();    
    bottom_layout->addWidget(buttonWidget);
    buttonWidget->setStyleSheet("background-color: rgb(207,214,229);");

    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(3, 9, 3, 9);

    // 创建并添加按钮    
    QToolButton *button_camera = new QToolButton();
    QToolButton *button_flow = new QToolButton();
    QToolButton *button_model = new QToolButton();
    QToolButton *button_comm = new QToolButton();
    QToolButton *button_setting = new QToolButton();
    QToolButton* button_log = new QToolButton();
    
    button_camera->setText("相机");
    button_camera->setIcon(icon("camera.png"));
    button_flow->setText("流程");
    button_flow->setIcon(icon("flow.png"));
    button_model->setText("模型");
    button_model->setIcon(icon("model.png"));
    button_comm->setText("通信");
    button_comm->setIcon(icon("comm.png"));
    button_setting->setText("设置");    
    button_setting->setIcon(icon("setting.png"));
    button_log->setText("事件");
    
    connect(button_camera,&QToolButton::clicked,this, &MainWindow::onBtnCamera);
    connect(button_flow, &QToolButton::clicked, this, &MainWindow::onBtnFlow);
    connect(button_model, &QToolButton::clicked, this, &MainWindow::onBtnModel);
    connect(button_comm, &QToolButton::clicked, this, &MainWindow::onBtnComm);
    connect(button_setting, &QToolButton::clicked, this, &MainWindow::onBtnSetting);
    connect(button_log, &QToolButton::clicked, this, &MainWindow::onBtnLog);

    QList<QToolButton*> btn_list;
    btn_list << button_camera;
    btn_list << button_flow;
    btn_list << button_model;
    btn_list << button_comm;
    btn_list << button_setting;
    for (int i = 0; i < btn_list.size(); i++)
    {
        buttonLayout->addWidget(btn_list[i]);        
        btn_list[i]->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn_list[i]->setIconSize(QSize(32, 32));        
    }
    buttonLayout->addStretch(1);                                    

    QWidget *line = new QWidget();
    line->setFixedWidth(3);
    line->setStyleSheet("background-color: rgb(41,57,85);");
    bottom_layout->addWidget(line);

    m_stack = new QStackedWidget();
    bottom_layout->addWidget(m_stack);

    addCameraPage();
    addFlowPage();
    addModelPage();
    addCommPage();
    addSettingPage();

    QWidget *bottom_widget = new QWidget();
    bottom_widget->setLayout(bottom_layout);
    bottom_widget->setObjectName("BottomWidget");
    bottom_widget->setStyleSheet("#BottomWidget { border: 6px solid rgb(41,57,85); } ");
    mainLayout->addWidget(bottom_widget);
}

QMenuBar *MainWindow::createMenuBar()
{
    QMenuBar *menubar = new QMenuBar();

    QMenu *menu_file = menubar->addMenu("文件");
    auto actNewMenu = menu_file->addAction("新建工程");
    auto actOpenMenu = menu_file->addAction("打开工程");
    connect(actNewMenu, &QAction::triggered, this, &MainWindow::onActionNewProject);
    connect(actOpenMenu, &QAction::triggered, this, &MainWindow::onActionOpenProject);

    QMenu *menu_help = menubar->addMenu("帮助");
    auto actHelp = menu_help->addAction("查看帮助");
    menu_help->addSeparator();    
    auto actAbout = menu_help->addAction("关于" + windowTitle());    
    connect(actHelp, &QAction::triggered, this, &MainWindow::onActionHelp);

    return menubar;
}

QWidget *MainWindow::createTitleBar()
{
    QHBoxLayout *l = new QHBoxLayout();

    QWidget *title_bar = new QWidget();
    title_bar->setLayout(l);

    JZTitleWidget *title = new JZTitleWidget();
    title->setFixedWidth(300);
    title->setFixedHeight(50);
    l->addWidget(title);            

    l->addStretch();

    title_bar->setStyleSheet("background-color: rgb(214,219,233);");

    return title_bar;
}


void MainWindow::addCameraPage()
{
    JZPanelWidget* panel = new JZPanelWidget();
    panel->addTab("设备列表", m_list);

    QSplitter* splitterTop = new QSplitter(Qt::Horizontal);
    splitterTop->addWidget(panel);
    splitterTop->addWidget(m_view);
    splitterTop->setSizes({ 200,600 });
    splitterTop->setStretchFactor(0, 0);
    splitterTop->setStretchFactor(1, 1);

    splitterTop->setChildrenCollapsible(false);
    setSliderStyle(splitterTop);
    m_stack->addWidget(splitterTop);
}

void MainWindow::addFlowPage()
{
    m_projectTree = new JZProjectTree();

    JZPanelWidget* panel = new JZPanelWidget();
    panel->addTab("流程列表", m_projectTree);

    QSplitter* splitterTop = new QSplitter(Qt::Horizontal);
    splitterTop->addWidget(panel);
    splitterTop->addWidget(new QWidget());
    splitterTop->setSizes({ 200,600 });
    splitterTop->setStretchFactor(0, 0);
    splitterTop->setStretchFactor(1, 1);

    splitterTop->setChildrenCollapsible(false);
    setSliderStyle(splitterTop);
    m_stack->addWidget(splitterTop);

    m_log = new LogWidget();
    connect(m_log, &LogWidget::sigNavigate, this, &MainWindow::onNavigate);

    connect(m_projectTree, &JZProjectTree::sigActionTrigged, this, &MainWindow::onProjectTreeAction);

    QWidget* widget = new QWidget();
    QVBoxLayout* center = new QVBoxLayout();
    center->setContentsMargins(9, 9, 9, 9);
    widget->setLayout(center);

    QWidget* widget_left = new QWidget();
    QVBoxLayout* l_left = new QVBoxLayout();
    l_left->setContentsMargins(0, 0, 0, 0);
    widget_left->setLayout(l_left);

    //main
    QSplitter* splitterMain = new QSplitter(Qt::Horizontal);
    splitterMain->setObjectName("splitterMain");
    splitterMain->addWidget(m_projectTree);
    splitterMain->addWidget(widget_left);

    center->addWidget(splitterMain);

    //left
    JZNodeEditor* node_editor = new JZNodeEditor();
    m_editorStack = new QTabWidget();
    m_editorStack->setTabsClosable(true);
    connect(m_editorStack, &QTabWidget::tabCloseRequested, this, &MainWindow::onEditorClose);
    connect(m_editorStack, &QTabWidget::currentChanged, this, &MainWindow::onEditorActivity);

    m_editorStack->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editorStack->tabBar(), &QWidget::customContextMenuRequested, this, &MainWindow::onTabContextMenu);

    QSplitter* splitterLeft = new QSplitter(Qt::Vertical);
    splitterLeft->addWidget(m_editorStack);
    splitterLeft->addWidget(m_log);
    l_left->addWidget(splitterLeft);

    splitterMain->setCollapsible(0, false);
    splitterMain->setCollapsible(1, false);
    splitterMain->setStretchFactor(0, 0);
    splitterMain->setStretchFactor(1, 1);
    splitterMain->setSizes({ 250,600 });

    splitterLeft->setCollapsible(0, false);
    splitterLeft->setCollapsible(1, false);
    splitterLeft->setStretchFactor(0, 1);
    splitterLeft->setStretchFactor(1, 0);

    m_stack->addWidget(widget);
}

void MainWindow::addModelPage()
{
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    QWidget* w = new QWidget();
    w->setLayout(l);

    QLabel* title = new QLabel("模型列表");
    l->addWidget(title);

    JZModelConfigWidget* config = new JZModelConfigWidget();
    l->addWidget(config);
    m_stack->addWidget(w);
}

void MainWindow::addCommPage()
{
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    QWidget* w = new QWidget();
    w->setLayout(l);

    QLabel* title = new QLabel("模型列表");
    l->addWidget(title);

    JZCommConfigWidget* config = new JZCommConfigWidget();
    l->addWidget(config);
    m_stack->addWidget(w);
}

void MainWindow::addLogPage()
{
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    QWidget* w = new QWidget();
    w->setLayout(l);

    QLabel* title = new QLabel("日志列表");
    l->addWidget(title);

    JZLogWidget* log = new JZLogWidget();
    l->addWidget(log);
    m_stack->addWidget(w);
}

void MainWindow::addSettingPage()
{
    QWidget* w = new QWidget();
    m_stack->addWidget(w);
}

void MainWindow::onBtnCamera()
{
    m_stack->setCurrentIndex(0);
}

void MainWindow::onBtnFlow()
{
    m_stack->setCurrentIndex(1);
}

void MainWindow::onBtnModel()
{
    m_stack->setCurrentIndex(2);
}

void MainWindow::onBtnComm()
{
    m_stack->setCurrentIndex(3);
}

void MainWindow::onBtnSetting()
{
    m_stack->setCurrentIndex(4);
}

void MainWindow::onBtnLog()
{
    m_stack->setCurrentIndex(5);
}

void MainWindow::onActionNewProject()
{
    if (!closeProject())
        return;

    JZNewProjectDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString name = dialog.name();
    QString project_dir = dialog.dir() + "/" + name;
    if (!QDir().exists(project_dir))
        QDir().mkpath(project_dir);

    QString project_path = project_dir + "/" + name + ".jzproj";
    QString project_tmp = dialog.projectType();
    
    initProject();
    if (!m_project.saveAs(project_path) || !m_project.saveAllItem())
    {
        QMessageBox::information(this, "", "新建工程失败");
        return;
    }
    openProject(project_path);
}

void MainWindow::onActionOpenProject()
{
    QString filepath = QFileDialog::getOpenFileName(this, "", "", "*.jzproj");
    if (filepath.isEmpty())
        return;

    if (!closeProject())
        return;

    openProject(filepath);
}

void MainWindow::onActionCloseProject()
{
    closeProject();
}

void MainWindow::onActionRecentProject()
{
    QAction* act = qobject_cast<QAction*>(sender());
    QString filepath = act->text();
    if (!closeProject())
        return;

    if (!openProject(filepath))
    {
        m_setting.recentFile.removeAll(filepath);
        QMenu* menu = qobject_cast<QMenu*>(act->parent());
        menu->removeAction(act);
        act->deleteLater();
    }
}

void MainWindow::onActionSaveFile()
{
    if (!m_editor)
        return;

    m_editor->save();
    updateActionStatus();
}

void MainWindow::onActionCloseFile()
{
    if (!m_editor)
        return;

    closeEditor(m_editor);
    updateActionStatus();
}

void MainWindow::onActionSaveAllFile()
{
    saveAll();
    updateActionStatus();
}

void MainWindow::onActionCloseAllFile()
{
    closeAllEditor();
}

void MainWindow::onActionCloseAllFileExcept()
{
    closeAllEditor(m_editor);
}

void MainWindow::onActionHelp()
{

}

void MainWindow::onEditorClose(int index)
{
    JZEditor* editor = qobject_cast<JZEditor*>(m_editorStack->widget(index));
    closeEditor(editor);
}

void MainWindow::onEditorActivity(int index)
{
    if (index == -1)
        return;

    JZEditor* editor = qobject_cast<JZEditor*>(m_editorStack->widget(index));
    switchEditor(editor);
}

void MainWindow::onNavigate(QUrl url)
{
    QString path = url.path();
    if (openEditor(path))
    {
        m_editor->navigate(url);
    }
}

void MainWindow::onCameraConfigChanged()
{
    JZCameraManagerConfig cfg = m_cameraManager->config();
}

void MainWindow::onProjectItemChanged(JZProjectItem* item)
{
    //editor
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        int index = m_editorStack->indexOf(it.value());
        updateTabText(index);

        it++;
    }
    onProjectChanged();
}

void MainWindow::onProjectChanged()
{
    m_task.addAutoCompilerTask();

    //editor
    auto list = nodeEditorList();
    for (auto node_edit : list)
        node_edit->updateDefine();
}

void MainWindow::onProjectTreeAction(int type, QString filepah)
{
    if (type == Action_open)
        openItem(filepah);
    else if (type == Action_close)
        closeItem(filepah);
    else if (type == Action_remove)
        removeItem(filepah);
}

void MainWindow::onTabContextMenu(QPoint pos)
{
    QMenu menu(this);
    QAction* actSave = menu.addAction("保存");
    QAction* actClose = menu.addAction("关闭");
    QAction* actAll = menu.addAction("关闭所有文档");
    QAction* actAllExcept = menu.addAction("除此之外全部关闭");

    auto bar = qobject_cast<QTabBar*>(sender());
    QAction* ret = menu.exec(bar->mapToGlobal(pos));
    if (!ret)
        return;
    if (ret == actSave)
    {
        onActionSaveFile();
    }
    else if (ret == actClose)
    {
        onActionCloseFile();
    }
    else if (ret == actAll)
    {
        onActionCloseAllFile();
    }
    else if (ret == actAllExcept)
    {
        int index = bar->tabAt(pos);
        auto editor = qobject_cast<JZEditor*>(m_editorStack->widget(index));
        closeAllEditor(editor);
    }
}

void MainWindow::onFlowRun()
{
}

void MainWindow::onFlowRunOnce()
{
}

void MainWindow::onFlowStop()
{
}

void MainWindow::initProject()
{
    m_project.clear();

    JZScriptClassItem *class_item = m_project.mainFile()->addClass("Appliaction","JZVisionPlatform");
    JZFunctionDefine func_init_def = class_item->objectDefine().initMemberFunction("init");
    JZScriptItem *script = class_item->addMemberFunction(func_init_def);
    auto start = script->startNode();
    JZNodeCameraInit * camera_init = new JZNodeCameraInit();
    JZNodeCommInit* comm_init = new JZNodeCommInit();
    JZNodeModelInit* model_init = new JZNodeModelInit();
    script->addNode(camera_init);
    script->addNode(model_init);
    script->addNode(comm_init);
    script->addConnect(start->flowOutGemo(), camera_init->flowInGemo());
    script->addConnect(camera_init->flowOutGemo(), model_init->flowInGemo());
    script->addConnect(model_init->flowOutGemo(), comm_init->flowInGemo());

    JZEditorUtils::projectUpdateLayout(&m_project);
}

bool MainWindow::closeProject()
{
    if (!closeAllEditor())
        return false;

    m_log->clearLogs();
    m_projectTree->clear();
    m_project.close();
    updateActionStatus();
    setWindowTitle("JZNodeEditor");
    return true;
}

bool MainWindow::openProject(QString filepath)
{
    if (!m_project.open(filepath))
    {
        QMessageBox::information(this, "", "打开工程失败: " + m_project.error());
        return false;
    }

    m_projectTree->setProject(&m_project);
    updateActionStatus();
    setWindowTitle(m_project.name());
    return true;
}

void MainWindow::openItem(QString filepath)
{
    openEditor(filepath);
}

void MainWindow::closeItem(QString filepath)
{
    auto edit = editor(filepath);
    if (!edit)
        return;

    closeEditor(edit);
}

void MainWindow::removeItem(QString itempath)
{
    auto item = m_project.getItem(itempath);
    auto file_item = m_project.getItemFile(item);

    closeItem(itempath);
    m_project.removeItem(itempath);
}

JZEditor* MainWindow::createEditor(int type)
{
    JZEditor* editor = JZEditorManager::instance()->createEditor(type);
    if (editor)
    {
        editor->setMainWindow(this);
        editor->setProject(&m_project);
    }
    return editor;
}

bool MainWindow::openEditor(QString filepath)
{
    if (filepath == "__idle__")
        return false;

    JZProjectItem* item = m_project.getItem(filepath);
    if (!item)
        return false;

    QString file = item->itemPath();
    auto new_edit = editor(file);
    if (!new_edit) {
        new_edit = createEditor(item->itemType());
        if (!new_edit)
            return false;
/*
        connect(new_edit, &JZEditor::redoAvailable, this, &MainWindow::onRedoAvailable);
        connect(new_edit, &JZEditor::undoAvailable, this, &MainWindow::onUndoAvailable);
        connect(new_edit, &JZEditor::modifyChanged, this, &MainWindow::onModifyChanged);
        new_edit->setItem(item);
        new_edit->open(item);
        if (new_edit->type() == ProjectItem_scriptItem)
        {
            auto node_edit = (JZNodeEditor*)new_edit;
            connect(node_edit, &JZNodeEditor::sigFunctionOpen, this, &MainWindow::onFunctionOpen);
            connect(node_edit, &JZNodeEditor::sigAutoCompiler, this, &MainWindow::onAutoCompiler);
            connect(node_edit, &JZNodeEditor::sigAutoRunOnce, this, &MainWindow::onAutoRunOnce);
            connect(node_edit, &JZNodeEditor::sigAutoRun, this, &MainWindow::onAutoRun);
            connect(node_edit, &JZNodeEditor::sigAutoRunStop, this, &MainWindow::onAutoRunStop);
            connect(node_edit, &JZNodeEditor::sigRuntimeValueChanged, this, &MainWindow::onEditorValueChanged);

            node_edit->setRunningMode(m_processMode);
            auto cmp_ret = compilerResult(file);
            if (cmp_ret)
                node_edit->setCompilerResult(cmp_ret);
        }
*/
        m_editors[item] = new_edit;
        m_editorStack->addTab(new_edit, filepath);
    }
    switchEditor(new_edit);

    return true;
}

void MainWindow::closeEditor(JZEditor* editor)
{
    auto item = editor->item();
    if (editor->isModified())
    {
        int ret = QMessageBox::question(this, "", "是否保存", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Yes)
        {
            editor->save();
        }
        else if (ret == QMessageBox::No)
        {
        }
        else if (ret == QMessageBox::Cancel)
        {
            return;
        }
    }
    editor->close();
    editor->setItem(nullptr);

    m_editors.remove(item);
    if (m_editor == editor)
    {
        if (m_editors.size() > 0)
            switchEditor(m_editors.first());
        else
            switchEditor(nullptr);
    }

    int index = m_editorStack->indexOf(editor);
    m_editorStack->removeTab(index);
    delete editor;
}

JZEditor* MainWindow::editor(QString filepath)
{
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        if (it.key()->itemPath() == filepath)
            return it.value();

        it++;
    }
    return nullptr;
}

JZNodeEditor* MainWindow::currentNodeEditor()
{
    if (m_editor && m_editor->type() == ProjectItem_scriptItem)
        return dynamic_cast<JZNodeEditor*>(m_editor);
    else
        return nullptr;
}

QList<JZNodeEditor*> MainWindow::nodeEditorList()
{
    QList<JZNodeEditor*> list;
    //editor
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (it.value()->type() == ProjectItem_scriptItem)
        {
            auto node_edit = (JZNodeEditor*)it.value();
            list << node_edit;
        }
        it++;
    }
    return list;
}

JZNodeEditor* MainWindow::nodeEditor(QString filepath)
{
    JZEditor* e = editor(filepath);
    if (!e)
        return nullptr;

    return qobject_cast<JZNodeEditor*>(e);
}

void MainWindow::switchEditor(JZEditor* editor)
{
    if (editor == m_editor)
        return;

    if (m_editor)
        m_editor->inactive‌();

    m_editor = editor;
    if (editor != nullptr)
    {
        m_editorStack->setCurrentWidget(m_editor);
        m_editor->active();
        m_editor->setFocus();
    }
    else
        m_editorStack->setCurrentIndex(0);
    updateActionStatus();
}

bool MainWindow::closeAllEditor(JZEditor* except)
{
    QList<JZEditor*> close_list;

    m_project.saveTransaction();
    bool saveToAll = false, noToAll = false;
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (except && except == editor)
        {
            it++;
            continue;
        }

        if (editor->isModified())
        {
            if (!saveToAll && !noToAll)
            {
                int ret = QMessageBox::question(this, "", "是否保存", QMessageBox::Yes | QMessageBox::No
                    | QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel);
                if (ret == QMessageBox::Yes || ret == QMessageBox::YesToAll)
                {
                    editor->save();
                    if (ret == QMessageBox::YesToAll)
                        saveToAll = true;
                }
                else if (ret == QMessageBox::No || ret == QMessageBox::NoToAll)
                {
                    if (ret == QMessageBox::NoToAll)
                        noToAll = true;
                }
                else if (ret == QMessageBox::Cancel)
                {
                    m_project.saveCommit();
                    return false;
                }
            }
            else if (saveToAll)
            {
                editor->save();
            }
            else if (noToAll)
            {

            }
        }
        editor->close();
        close_list << editor;

        it++;
    }
    m_project.saveCommit();

    for (auto editor : close_list)
    {
        int index = m_editorStack->indexOf(editor);
        m_editorStack->removeTab(index);
        m_editors.remove(editor->item());
        delete editor;
    }

    m_editor = nullptr;
    switchEditor(except);
    return true;
}

void MainWindow::saveAll()
{
    m_project.saveTransaction();
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (editor->isModified())
            editor->save();

        it++;
    }
    m_project.saveCommit();
}

void MainWindow::updateActionStatus()
{

}

void MainWindow::updateTabText(int index)
{
    auto editor = qobject_cast<JZEditor*>(m_editorStack->widget(index));
    QString title = editor->item()->itemPath();
    if (editor->isModified())
        title += "*";
    m_editorStack->setTabText(index, title);
}