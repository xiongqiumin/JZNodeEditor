#ifndef JZ_COMM_SIMULATOR_H_
#define JZ_COMM_SIMULATOR_H_

#include <QMdiArea>
#include <QMainWindow>
#include <QTreeWidget>
#include <QTableWidget>
#include <QPlainTextEdit>
#include "UiCommon.h"
#include "JZCommManager.h"
#include "jzWidgets/JZLogWidget.h"
#include "JZCommSimulatorWidget.h"

class SimulatorWidget;

class JZCommSimulatorWidgetConfig
{
public:
    int type;
    QByteArray buffer;
};

class JZCommSimulatorConfig
{
public:      
    QList<JZCommSimulatorWidgetConfig> commList;
};
QDataStream &operator<<(QDataStream &s, const JZCommSimulatorConfig &param);
QDataStream &operator>>(QDataStream &s, JZCommSimulatorConfig &param);

//这里使用 mainwindow ， 因为mdiarea 放大窗口会和菜单重合
class JZCommSimulator : public QWidget
{
    Q_OBJECT

public:
    JZCommSimulator(QWidget *parent = nullptr);
    virtual ~JZCommSimulator();    

    void setConfig(JZCommSimulatorConfig config);
    JZCommSimulatorConfig config();

    void closeAll();

signals:
    void sigClose();

protected slots :
    void onActionNew();
    void onActionClear();
    void onActionShowAll();
    void onContextMenu(QPoint pt);    

    void onItemDoubleClicked(QTreeWidgetItem *item);

    void onActionSaveConfig();
    void onActionLoadConfig();

protected:
    struct Simulator
    {
        QTreeWidgetItem *item;
        QMdiSubWindow *window;        
        JZCommSimulatorWidget *widget;
        JZCommSimulatorWidgetConfig config;
    };
    
    virtual void closeEvent(QCloseEvent *event) override;
    virtual bool eventFilter(QObject *o, QEvent *e) override;

    JZCommSimulatorWidget *createSimulator(int type);
    void addSimulator(JZCommSimulatorWidgetConfig config);
    void removeSimulator(int index);
    void startSimulator(int index);
    void stopSimulator(int index);
    void settingSimulator(int index);    
    QString genSimulatorName(int index);    
                
    QList<Simulator> m_simulator;
    QMdiArea *m_mdiArea;
    JZLogWidget *m_log;
    QTreeWidget *m_tree;    
};

#endif
