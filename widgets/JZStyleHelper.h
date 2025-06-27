#ifndef JZ_STYLE_HELPER_H_
#define JZ_STYLE_HELPER_H_

#include <QString>
#include <QColor>

class JZStypeClass
{
public:
    JZStypeClass();

    void setObjectName(const QString& objectName);
    void setClassName(const QString& className);
    void setBackgroundColor(const QColor& color);
    void setForegroundColor(const QColor& color);

    QString styleSheet() const;

private:
    QString colorToString(const QColor& color) const;
    QString m_backgroundColor;
    QString m_foregroundColor;

    QString m_className;
    QString m_objectName;
};

#endif // !JZ_STYLE_HELPER_H_
