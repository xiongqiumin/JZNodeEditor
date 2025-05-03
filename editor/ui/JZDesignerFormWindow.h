#ifndef JZ_DESIGNER_FORMWINDOW_H
#define JZ_DESIGNER_FORMWINDOW_H

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>
#include <QScrollArea>

QT_BEGIN_NAMESPACE


namespace SharedTools { namespace Internal{ class FormResizer; }}

class JZDesignerEditor;
class QDesignerFormWindowInterface;
class JZDesignerFormWindow : public QScrollArea
{
    Q_OBJECT

public:
    explicit JZDesignerFormWindow(QWidget *parent = 0, QDesignerFormWindowInterface *formWindow = 0);
    virtual ~JZDesignerFormWindow();
    // Show handles if active and main container is selected.
    void updateFormWindowSelectionHandles(bool active);

    inline QDesignerFormWindowInterface *editor() const { return m_formWindow; }

    QWidget *integrationContainer() const;

protected:
    void setFormWindow(QDesignerFormWindowInterface *fw);

signals:
    void formWindowSizeChanged(int, int);

    private slots:
    void fwSizeWasChanged(const QRect &, const QRect &);

private:
    QSize formWindowSize() const;

    QDesignerFormWindowInterface *m_formWindow;
    SharedTools::Internal::FormResizer *m_formResizer;
    QSize m_oldFakeWidgetSize;
};

QT_END_NAMESPACE

#endif // QDESIGNER_FORMWINDOW_H
