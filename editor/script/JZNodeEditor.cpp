#include "JZNodeEditor.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QDebug>
#include <QLabel>
#include <QDialogButtonBox>
#include <QTabWidget>
#include "JZNodeFunctionManager.h"
#include "JZNodeFactory.h"
#include "JZNodeMemberEditDialog.h"

//JZListInitFunct
bool JZListInitFunction(JZNode *node)
{
    QString value = node->paramInValue(0).toString();
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
    JZNodeMemberEditDialog dialog(node->file()->editor());
    dialog.init(node);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    return true;
}

//JZNodeEditor
JZNodeEditor::JZNodeEditor()
{
    m_type = Editor_script;
    init();           

    auto func_inst = JZNodeFunctionManager::instance();
    func_inst->registEditFunction("StringList.create", JZListInitFunction);
    func_inst->registEditFunction("List.create", JZListInitFunction);
    func_inst->registEditFunction("Map.create", JZListInitFunction);

    JZNodeFactory::instance()->registEdit(Node_memberParam, JZMemberEdit);
    JZNodeFactory::instance()->registEdit(Node_setMemberParam, JZMemberEdit);    
}

JZNodeEditor::~JZNodeEditor()
{

}

void JZNodeEditor::init()
{
    m_view = new JZNodeView();
    m_nodePanel = new JZNodePanel();
    m_nodeViewPanel = new JZNodeViewPanel();
    m_nodeProp = new JZNodePropertyEditor();
    connect(m_view,&JZNodeView::redoAvailable,this,&JZNodeEditor::redoAvailable);
    connect(m_view,&JZNodeView::undoAvailable,this,&JZNodeEditor::undoAvailable);
    connect(m_view,&JZNodeView::modifyChanged,this,&JZNodeEditor::modifyChanged);
    connect(m_view,&JZNodeView::sigFunctionOpen, this, &JZNodeEditor::sigFunctionOpen);

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    this->setLayout(l);    
    
    QTabWidget *m_tabView = new QTabWidget();
    m_tabView->addTab(m_nodePanel,"编辑");
    m_tabView->addTab(m_nodeViewPanel,"节点");
    m_tabView->setTabPosition(QTabWidget::South);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);    
    splitter->addWidget(m_tabView);
    splitter->addWidget(m_view);
    splitter->addWidget(m_nodeProp);

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
}

void JZNodeEditor::open(JZProjectItem *item)
{
    JZScriptFile* file = dynamic_cast<JZScriptFile*>(item);
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

void JZNodeEditor::setNodeValue(int nodeId, int prop_id, const QString &value)
{
    m_view->setNodePropValue(nodeId, prop_id, value);
}