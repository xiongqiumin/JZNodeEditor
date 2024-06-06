#include "JZNodeEditor.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QDebug>
#include <QLabel>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QCheckBox>
#include <QPushButton>
#include "JZNodeFunctionManager.h"
#include "JZNodeFactory.h"
#include "JZNodeMemberSelectDialog.h"
#include "LogManager.h"

//JZListInitFunct
bool JZListInitFunction(JZNode *node)
{
    QString value = node->paramInValue(0);
    value.replace(",", "\n");
    QDialog dialog(node->file()->editor());

    QVBoxLayout *l = new QVBoxLayout();
    l->addWidget(new QLabel("init"));

    QTextEdit *edit = new QTextEdit();
    edit->setPlainText(value);
    l->addWidget(edit);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
        | QDialogButtonBox::Cancel);
    dialog.connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    dialog.connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    l->addWidget(buttonBox);
    dialog.setLayout(l);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    QStringList result;
    QStringList lines = edit->toPlainText().split("\n");
    for (int i = 0; i < lines.size(); i++)
    {
        QString line = lines[i].simplified();
        if (!line.isEmpty())
            result.push_back(line);
    }
    node->setParamInValue(0, result.join(","));
    return true;
}

//JZMemberEdit
bool JZMemberEdit(JZNode *node)
{
    JZNodeMemberSelectDialog dialog(node->file()->editor());
    dialog.init(node);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    return true;
}

//JZNodeEditor
JZNodeEditor::JZNodeEditor()
{
    m_type = Editor_script;    
}

JZNodeEditor::~JZNodeEditor()
{

}

QWidget *JZNodeEditor::createMidBar()
{    
    QWidget *mid_bar = new QWidget();
    mid_bar = new QWidget();

    QHBoxLayout *midbar_layout = new QHBoxLayout();
    mid_bar->setLayout(midbar_layout);
    midbar_layout->setContentsMargins(0, 0, 0, 0);

    if (script()->isFunction())
    {        
        QCheckBox *boxAuto = new QCheckBox("开启测试");
        connect(boxAuto, &QCheckBox::clicked, this, &JZNodeEditor::onAutoRunChecked);

        QPushButton *btnAutoRun = new QPushButton("运行");
        connect(btnAutoRun, &QPushButton::clicked, this, &JZNodeEditor::onAutoRuning);

        midbar_layout->addWidget(boxAuto);
        midbar_layout->addWidget(btnAutoRun);
        midbar_layout->addStretch();                
    }

    return mid_bar;
}

void JZNodeEditor::init()
{                
    //left
    m_nodePanel = new JZNodePanel();
    m_nodeViewPanel = new JZNodeViewPanel();

    QTabWidget *tabView = new QTabWidget();
    tabView->addTab(m_nodePanel, "编辑");
    tabView->addTab(m_nodeViewPanel, "节点");
    tabView->setTabPosition(QTabWidget::South);

    //mid
    QWidget *mid_bar = createMidBar();
    
    m_view = new JZNodeView();
    connect(m_view, &JZNodeView::redoAvailable, this, &JZNodeEditor::redoAvailable);
    connect(m_view, &JZNodeView::undoAvailable, this, &JZNodeEditor::undoAvailable);
    connect(m_view, &JZNodeView::modifyChanged, this, &JZNodeEditor::modifyChanged);
    connect(m_view, &JZNodeView::sigFunctionOpen, this, &JZNodeEditor::sigFunctionOpen);
    connect(m_view, &JZNodeView::sigAutoCompiler, this, &JZNodeEditor::sigAutoCompiler);
    connect(m_view, &JZNodeView::sigAutoRun, this, &JZNodeEditor::sigAutoRun);

    QWidget *mid_widget = new QWidget();
    QVBoxLayout *mid_layout = new QVBoxLayout();
    mid_widget->setLayout(mid_layout);
    mid_layout->addWidget(mid_bar);
    mid_layout->addWidget(m_view);
    mid_layout->setContentsMargins(0, 0, 0, 0);        

    //right
    m_nodeProp = new JZNodePropertyEditor();
    m_runProp = new JZNodeAutoRunWidget();

    m_tabProp = new QTabWidget();
    m_tabProp->addTab(m_nodeProp, "属性");
    m_tabProp->addTab(m_runProp, "运行配置");
    m_tabProp->setTabPosition(QTabWidget::South);    

    //init widgets
    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    this->setLayout(l);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);    
    splitter->addWidget(tabView);
    splitter->addWidget(mid_widget);
    splitter->addWidget(m_tabProp);

    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);
    splitter->setCollapsible(2,false);
    splitter->setStretchFactor(0,0);
    splitter->setStretchFactor(1,1);
    splitter->setStretchFactor(2,0);
    splitter->setSizes({220,300,220});
    l->addWidget(splitter);
    
    //m_nodeProp->setMaximumWidth(200);
    m_view->setPropertyEditor(m_nodeProp);    
    m_view->setRunEditor(m_runProp);

    //todo:
    //m_tabProp->setTabVisible(1, false);
}    

void JZNodeEditor::open(JZProjectItem *item)
{
    init();

    JZScriptItem* file = dynamic_cast<JZScriptItem*>(item);    
    m_view->setFile(file);    
    m_nodePanel->setFile(file);    
}

void JZNodeEditor::close()
{

}

void JZNodeEditor::save()
{    
    m_view->save();
}

void JZNodeEditor::addMenuBar(QMenuBar *menubar)
{
    QMenu *menu = menubar->actions()[Menu_View]->menu();
    m_actionList << menu->addSeparator();
    m_actionList << menu->addAction("自动布局");
    m_actionList << menu->addAction("显示全部");
    connect(m_actionList[1], &QAction::triggered, this, &JZNodeEditor::onActionLayout);
    connect(m_actionList[2], &QAction::triggered, this, &JZNodeEditor::onActionFitInView);
}

void JZNodeEditor::removeMenuBar(QMenuBar *menubar)
{
    QMenu *menu = menubar->actions()[Menu_View]->menu();
    for (int i = 0; i < m_actionList.size(); i++)
    {
        menu->removeAction(m_actionList[i]);
        delete m_actionList[i];
    }
    m_actionList.clear();
}

bool JZNodeEditor::isModified()
{
    return m_view->isModified();
}

void JZNodeEditor::undo()
{
    m_view->undo();
}

void JZNodeEditor::redo()
{
    m_view->redo();
}

void JZNodeEditor::remove()
{
    m_view->remove();
}

void JZNodeEditor::cut()
{
    m_view->cut();
}

void JZNodeEditor::copy()
{
    m_view->copy();
}

void JZNodeEditor::paste()
{
    m_view->paste();
}

void JZNodeEditor::selectAll()
{
    m_view->selectAll();
}

void JZNodeEditor::onAutoRunChecked()
{
    auto *box = qobject_cast<QCheckBox*>(sender());
    m_tabProp->setTabVisible(1, box->isChecked());
    m_view->setAutoRunning(box->isChecked());   
}

void JZNodeEditor::onAutoRuning()
{
    auto script = this->script();    
    if (m_runProp->depend().function.fullName() != script->function().fullName())
        return;

    emit sigAutoRun();
}

void JZNodeEditor::onActionLayout()
{
    m_view->updateNodeLayout();
}

void JZNodeEditor::onActionFitInView()
{
    m_view->fitNodeView();
}

void JZNodeEditor::ensureNodeVisible(int nodeId)
{
    m_view->ensureNodeVisible(nodeId);
}

void JZNodeEditor::selectNode(int nodeId)
{
    m_view->selectNode(nodeId);
}

BreakPointTriggerResult JZNodeEditor::breakPointTrigger()
{
    return m_view->breakPointTrigger();
}

void JZNodeEditor::setRunning(bool status)
{
    m_view->setRunning(status);
}

int JZNodeEditor::runtimeNode()
{
    return m_view->runtimeNode();
}

void JZNodeEditor::setRuntimeNode(int nodeId)
{
    m_view->setRuntimeNode(nodeId);
}

void JZNodeEditor::updateNode()
{
    m_nodePanel->updateNode();
}

JZScriptItem *JZNodeEditor::script()
{
    JZScriptItem* file = dynamic_cast<JZScriptItem*>(m_item);
    return file;
}

ScriptDepend JZNodeEditor::scriptTestDepend()
{
    return m_runProp->depend();
}

void JZNodeEditor::setNodeValue(int nodeId, int prop_id, const QString &value)
{
    m_view->setNodePropValue(nodeId, prop_id, value);
}

void JZNodeEditor::setCompierResult(const CompilerInfo &info)
{
    m_view->setCompierResult(info);

    auto s = script();
    if (s->itemType() == ProjectItem_scriptFunction)
    {
        QString function = script()->function().fullName();
        m_runProp->setDepend(info.depend[function]);
    }
}

void JZNodeEditor::setAutoRunResult(const UnitTestResult &info)
{
    if(info.result)
    {
        LOGI(Log_Runtime, "运行完毕.");
        m_runProp->setResult(info.out);
    }
    else
    {
        LOGI(Log_Runtime, "run filed:" + info.runtimeError.error);
    }
}
