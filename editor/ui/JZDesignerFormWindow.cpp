#include "JZDesignerFormWindow.h"
#include "formresizer.h"
#include "widgethostconstants.h"

#include <QDesignerFormWindowInterface>
#include <QDesignerFormWindowCursorInterface>

#include <QPalette>
#include <QLayout>
#include <QFrame>
#include <QResizeEvent>
#include <QDebug>

using namespace SharedTools;

// ---------- JZDesignerFormWindow
JZDesignerFormWindow::JZDesignerFormWindow(QWidget *parent, QDesignerFormWindowInterface *formWindow) :
    QScrollArea(parent),
    m_formWindow(0),
    m_formResizer(new Internal::FormResizer)
{    
    setWidget(m_formResizer);
    // Re-set flag (gets cleared by QScrollArea): Make the resize grip of a mainwindow form find the resizer as resizable window.
    m_formResizer->setWindowFlags(m_formResizer->windowFlags() | Qt::SubWindow);
    setFormWindow(formWindow);
}

JZDesignerFormWindow::~JZDesignerFormWindow()
{
    if (m_formWindow)
        delete m_formWindow;
}

void JZDesignerFormWindow::setFormWindow(QDesignerFormWindowInterface *fw)
{
    m_formWindow = fw;
    if (!fw)
        return;

    m_formResizer->setFormWindow(fw);

    setBackgroundRole(QPalette::Base);
    m_formWindow->setAutoFillBackground(true);
    m_formWindow->setBackgroundRole(QPalette::Background);

    connect(m_formResizer, &Internal::FormResizer::formWindowSizeChanged,
        this, &JZDesignerFormWindow::fwSizeWasChanged);
}

QSize JZDesignerFormWindow::formWindowSize() const
{
    if (!m_formWindow || !m_formWindow->mainContainer())
        return QSize();
    return m_formWindow->mainContainer()->size();
}

void JZDesignerFormWindow::fwSizeWasChanged(const QRect &, const QRect &)
{
    // newGeo is the mouse coordinates, thus moving the Right will actually emit wrong height
    emit formWindowSizeChanged(formWindowSize().width(), formWindowSize().height());
}

void JZDesignerFormWindow::updateFormWindowSelectionHandles(bool active)
{
    Internal::SelectionHandleState state = Internal::SelectionHandleOff;
    const QDesignerFormWindowCursorInterface *cursor = m_formWindow->cursor();
    if (cursor->isWidgetSelected(m_formWindow->mainContainer()))
        state = active ? Internal::SelectionHandleActive : Internal::SelectionHandleInactive;

    m_formResizer->setState(state);
}

QWidget *JZDesignerFormWindow::integrationContainer() const
{
    return m_formResizer;
}
