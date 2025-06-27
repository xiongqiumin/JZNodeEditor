#include "JZStyleHelper.h"

JZStypeClass::JZStypeClass()
    : m_backgroundColor(), m_foregroundColor()
{
}

QString JZStypeClass::colorToString(const QColor& color) const
{
    return QString("rgb(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
}

void JZStypeClass::setObjectName(const QString& objectName)
{
    m_objectName = objectName;
}

void JZStypeClass::setClassName(const QString& className)
{
    m_className = className;
}

void JZStypeClass::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = colorToString(color);
}

void JZStypeClass::setForegroundColor(const QColor& color)
{
    m_foregroundColor = colorToString(color);
}

QString JZStypeClass::styleSheet() const
{
    QString styleSheet;

    if (!m_backgroundColor.isEmpty()) {
        styleSheet += QString("background-color: %1;").arg(m_backgroundColor);
    }

    if (!m_foregroundColor.isEmpty()) {
        styleSheet += QString("color: %1;").arg(m_foregroundColor);
    }

    // Ó¦ÓÃÑ¡ÔñÆ÷
    QString selector;
    if (!m_className.isEmpty()) {
        selector = m_className;
    }
    else if (!m_objectName.isEmpty()) {
        selector = "#" + m_objectName;
    }
    
    if (!selector.isEmpty())
        styleSheet = selector + " {" + styleSheet + "}";

    return styleSheet;
}