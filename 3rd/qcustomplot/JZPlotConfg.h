#ifndef  JZ_PLOT_CONFIG_H_
#define  JZ_PLOT_CONFIG_H_

#include <QLineEdit>
#include "JZNodeFunction.h"
#include "JZBaseDialog.h"
#include "qcustomplot.h"

enum {
    Node_plotConfig = 2000,
};

class JZPlotConfig
{
public:
    JZPlotConfig();

    int count;
};
QDataStream &operator<<(QDataStream &s, const JZPlotConfig &param);
QDataStream &operator>>(QDataStream &s, JZPlotConfig &param);

class JZPlotConfigDialog : public JZBaseDialog
{
public:
    JZPlotConfigDialog();
    ~JZPlotConfigDialog();

    void setConfig(JZPlotConfig config);
    JZPlotConfig config();

protected:
    virtual bool onOk() override;

    QLineEdit *m_lineCount;
};


class JZNodePlotConfig : public JZNodeFunctionCustom
{
public:
    JZNodePlotConfig();
    ~JZNodePlotConfig();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    virtual void initFunction() override;

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

protected:
    virtual JZNodePinWidget *createWidget(int id);    

    JZPlotConfig m_config;
};

class JZPlotWidget : public QWidget
{
    Q_OBJECT

public:
    JZPlotWidget();
    ~JZPlotWidget();

    void init(JZPlotConfig config);
    void clear();
    void addData(int index, double value);

protected slots:
    void realtimeDataSlot();

protected:
    double m_time;
    QCustomPlot *m_plot;
};

void InitCustomPlot();


#endif // ! JZ_PLOT_H_
