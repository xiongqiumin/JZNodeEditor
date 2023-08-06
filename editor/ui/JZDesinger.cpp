#include "JZDesinger.h"
#include <QtDesigner/QDesignerIntegrationInterface>
#include <QtDesigner/QDesignerSettingsInterface>
#include <QtDesigner/5.15.2/QtDesigner/private/shared_settings_p.h>
#include "JZDesignerEditor.h"
#include "JZNodeIntegration.h"

JZDesigner *JZDesigner::instance()
{
    static JZDesigner inst;
    return &inst;
}

JZDesigner::JZDesigner()
{
    QDesignerComponents::initializeResources();    
    m_core = nullptr;
    m_editor = nullptr;
}

JZDesigner::~JZDesigner()
{
    closeEditor();
}

JZDesignerEditor *JZDesigner::editor()
{
    if (!m_editor)
    {
        m_core = QDesignerComponents::createFormEditor(this);
        qdesigner_internal::QDesignerSharedSettings *setting = new qdesigner_internal::QDesignerSharedSettings(m_core);

        m_editor = new JZDesignerEditor();
        m_editor->init(m_core);

        JZNodeIntegration *d = new JZNodeIntegration(m_core);
        m_core->setIntegration(d);
    }
    return m_editor;
}

void JZDesigner::closeEditor()
{
    if (m_editor)
    {
        delete m_editor;
        m_editor = nullptr;
    }
}