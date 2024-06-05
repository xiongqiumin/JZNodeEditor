#include "JZDesignerEditor.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QDockWidget>
#include <QApplication>
#include <QMdiArea>
#include <QDebug>
#include <QMdiSubWindow>
#include "JZDesignerFormWindow.h"

JZDesignerFormWindow *JZDesignerEditor::m_active = nullptr;

JZDesignerFormWindow *JZDesignerEditor::activeWidgetHost()
{
    return m_active;
}

JZDesignerEditor::JZDesignerEditor()
{
    m_core = nullptr;
    m_empty = new QWidget(this);
}

JZDesignerEditor::~JZDesignerEditor()
{        
}   

QDesignerFormEditorInterface *JZDesignerEditor::core()
{
    return m_core;
}

void JZDesignerEditor::init(QDesignerFormEditorInterface *core)
{
    m_core = core;
    m_view = new QStackedWidget(this);
    core->setTopLevel(m_view);

    connect(m_view, &QStackedWidget::currentChanged,
        this, &JZDesignerEditor::slotSubWindowActivated);

    auto panel = QDesignerComponents::createWidgetBox(core, this); 
    panel->setFileName(":/JZNodeEditor/Resources/widgetBox.xml");

    auto objectInspector = QDesignerComponents::createObjectInspector(core, this);
    auto propEditor = QDesignerComponents::createPropertyEditor(core, this);
    core->setWidgetBox(panel);
    core->setObjectInspector(objectInspector);
    core->setPropertyEditor(propEditor);

    //right
    QWidget *right_widget = new QWidget();
    QVBoxLayout *r_v = new QVBoxLayout();
    r_v->setContentsMargins(0, 0, 0, 0);
    right_widget->setLayout(r_v);

    QDockWidget *r_w1 = new QDockWidget("对象查看器");
    QDockWidget *r_w2 = new QDockWidget("属性编辑器");
    r_w1->setFeatures(QDockWidget::NoDockWidgetFeatures);
    r_w2->setFeatures(QDockWidget::NoDockWidgetFeatures);

    r_w1->setWidget(objectInspector);
    r_w2->setWidget(propEditor);

    QSplitter *r_splitter = new QSplitter(Qt::Vertical);
    r_splitter->addWidget(r_w1);
    r_splitter->addWidget(r_w2);
    r_splitter->setCollapsible(0, false);
    r_splitter->setCollapsible(1, false);
    r_v->addWidget(r_splitter);

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(panel);
    splitter->addWidget(m_view);
    splitter->addWidget(right_widget);

    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    splitter->setCollapsible(2, false);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 0);
    splitter->setSizes({ 150,300,150 });
    l->addWidget(splitter);       
    this->setLayout(l);
}

JZDesignerFormWindow *JZDesignerEditor::open(JZUiFile *file)
{    
    QDesignerFormWindowInterface *form = m_core->formWindowManager()->createFormWindow();
    
    QString xml = file->xml();
    if (xml.isEmpty())
    {
        form->setMainContainer(new QWidget());
    }
    else
    {
        form->setContents(xml);
    }    

    JZDesignerFormWindow *window = new JZDesignerFormWindow(this, form);
    connect(window, &JZDesignerFormWindow::formWindowSizeChanged,
        this, &JZDesignerEditor::formSizeChanged);

    m_windows.push_back(window);
    m_view->addWidget(window);
    m_view->setCurrentWidget(window);    
    return window;
}

void JZDesignerEditor::close(JZDesignerFormWindow *window)
{        
    if (m_active == window)
        m_active = nullptr;

    m_view->removeWidget(window);
    m_windows.removeAll(window);
    delete window;
}

void JZDesignerEditor::showForm(JZDesignerFormWindow *form)
{    
    m_view->setCurrentWidget(form);
    core()->formWindowManager()->setActiveFormWindow(form->editor());
    m_active = form;
}

void JZDesignerEditor::hideForm(JZDesignerFormWindow *window)
{
    window->hide();
}

Qt::WindowFlags JZDesignerEditor::windowFlag()
{
    return Qt::Window | Qt::WindowShadeButtonHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint;
}

void JZDesignerEditor::slotSubWindowActivated(int index)
{          
    if(index >= 0)
        core()->formWindowManager()->setActiveFormWindow(m_windows[index]->editor());
}

void JZDesignerEditor::formSizeChanged(int w, int h)
{    
    if (const JZDesignerFormWindow *wh = qobject_cast<const JZDesignerFormWindow *>(sender())) {
        wh->editor()->setDirty(true);
        static const QString geometry = QLatin1String("geometry");
        m_core->propertyEditor()->setPropertyValue(geometry, QRect(0, 0, w, h));
    }
}