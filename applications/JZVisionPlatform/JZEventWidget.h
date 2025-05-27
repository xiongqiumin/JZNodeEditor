#ifndef JZEVENTWIDGET_H
#define JZEVENTWIDGET_H

#include <QWidget>
#include <QTableWidget>

class JZEventWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JZEventWidget(QWidget *parent = nullptr);
    ~JZEventWidget();

    void addEvent(const QString &time, const QString &type, const QString &description);
    void clearEvents();

private:
    QTableWidget *tableWidget;

    void setupUI();
};

#endif // JZEVENTWIDGET_H    