#ifndef JZPANELWIDGET_H
#define JZPANELWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QToolButton>
#include <QStackedWidget>
#include <QTimer>

class JZPanelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JZPanelWidget(QWidget *parent = nullptr);
    ~JZPanelWidget();

    // Ìí¼Ó±êÇ©Ò³
    void addTab(const QString &title, QWidget *widget);

signals:

private slots:
    void onToggleButtonClicked();
    void onHandleTimer();

private:    
    struct PanelInfo
    {
        QWidget *panel;
        QToolButton *btn;        
        QWidget *widget;
        int height;    //¿Ø¼þheight
    };
    
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;    
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;    

    int resizeHandleIndex(int y);
    void updateLayout();
    
    QList<PanelInfo> m_panelList;
    QTimer *m_handleTimer;
    
    int m_handleIndex;
    int m_preHeight;
    int m_curHeight;
    QPoint m_downPoint;
};

#endif // JZPANELWIDGET_H    