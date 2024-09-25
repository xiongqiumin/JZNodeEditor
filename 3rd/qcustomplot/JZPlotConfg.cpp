#include <QFormLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include <QTimer>
#include "JZPlotConfg.h"
#include "JZNodePinWidget.h"
#include "JZNodeCompiler.h"
#include "JZNodeEditorManager.h"
#include "JZNodeBind.h"
#include "JZNodeFactory.h"

//JZPlotConfig
JZPlotConfig::JZPlotConfig()
{
    count = 1;
}

QDataStream &operator<<(QDataStream &s, const JZPlotConfig &param)
{
    s << param.count;
    return s;
}
QDataStream &operator >> (QDataStream &s, JZPlotConfig &param)
{
    s >> param.count;
    return s;
}

//JZPlotConfigDialog
JZPlotConfigDialog::JZPlotConfigDialog()
{
    QFormLayout *form = new QFormLayout();
    m_lineCount = new QLineEdit();
    form->addRow("Line Number:", m_lineCount);

    m_mainWidget->setLayout(form);
}

JZPlotConfigDialog::~JZPlotConfigDialog()
{

}

void JZPlotConfigDialog::setConfig(JZPlotConfig config)
{
    m_lineCount->setText(QString::number(config.count));
}

JZPlotConfig JZPlotConfigDialog::config()
{
    JZPlotConfig config;
    config.count = m_lineCount->text().toInt();
    return config;
}

bool JZPlotConfigDialog::onOk()
{
    return true;
}

JZNodePlotConfig::JZNodePlotConfig()
{
    m_type = Node_plotConfig;
}

JZNodePlotConfig::~JZNodePlotConfig()
{

}

void JZNodePlotConfig::initFunction()
{
    clearPin();

    addFlowIn();
    addFlowOut();

    int in = addParamIn("plot", Pin_dispName);
    pin(in)->setDataType({ "JZPlotWidget" });

    addParamIn("plot", Pin_widget | Pin_noValue);
    setName(m_functionName);
}

JZNodePinWidget *JZNodePlotConfig::createWidget(int id)
{
    if (id == paramIn(1))
    {
        JZNodePinButtonWidget *w = new JZNodePinButtonWidget(this, id);
        auto btn = w->button();
        btn->setText("设置");
        btn->connect(btn, &QPushButton::clicked, [btn, this] {
            QByteArray old = this->toBuffer();

            JZPlotConfigDialog dlg;
            dlg.setConfig(m_config);
            if (dlg.exec() != QDialog::Accepted)
                return;

            m_config = dlg.config();
            propertyChangedNotify(old);
        });
        return w;
    }

    return nullptr;
}

bool JZNodePlotConfig::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << m_config;

    int obj_id = JZNodeGemo::paramId(m_id, paramIn(0));
    int id = c->allocStack(Type_byteArray);
    c->addSetBuffer(irId(id), buffer);

    QList<JZNodeIRParam> in, out;
    in << irId(obj_id) << irId(id);
    c->addCall(m_functionName, in, out);
    c->addJumpNode(flowOut());

    return true;
}

void JZNodePlotConfig::saveToStream(QDataStream &s) const
{
    JZNodeFunctionCustom::saveToStream(s);
    s << m_functionName << m_config;
}

void JZNodePlotConfig::loadFromStream(QDataStream &s)
{
    JZNodeFunctionCustom::loadFromStream(s);
    s >> m_functionName >> m_config;
}

//JZPlotWidget
JZPlotWidget::JZPlotWidget()
{
    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    m_plot = new QCustomPlot();
    l->addWidget(m_plot);
    setLayout(l);

    QTimer *timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    timer->start(50);

    m_time = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    m_plot->xAxis->setRange(0, 70);
    m_plot->yAxis->setRange(0, 100);
}

JZPlotWidget::~JZPlotWidget()
{
}

void JZPlotWidget::init(JZPlotConfig config)
{
    clear();
    for (int i = 0; i < config.count; i++)
        m_plot->addGraph();
}

void JZPlotWidget::clear()
{
    QVector<double> empty;
    for (int i = 0; i < m_plot->graphCount(); i++)
        m_plot->graph(i)->setData(empty, empty);

    m_time = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    m_plot->xAxis->setRange(0, 70);
    m_plot->yAxis->setRange(0, 100);
}

void JZPlotWidget::addData(int index, double value)
{
    if (m_plot->graphCount() < index)
        return;

    double ms = QDateTime::currentMSecsSinceEpoch()/1000.0 - m_time;
    m_plot->graph(index)->addData(ms,value);
    m_plot->yAxis->rescale();
    m_plot->replot();
}

void JZPlotWidget::realtimeDataSlot()
{
    double gap = QDateTime::currentMSecsSinceEpoch()/1000.0 - m_time;

    auto xAxis = m_plot->xAxis;
    if (gap <= 60)
        xAxis->setRange(0, 70);
    else
        xAxis->setRange(gap - 60, gap + 10);

    m_plot->replot();    
}

//initPlotConfig
void initPlotConfig(JZPlotWidget *plot, const QByteArray &buffer)
{
    JZPlotConfig config;
    QDataStream s(buffer);
    s >> config;
    plot->init(config);
}

//InitCustomPlot
void InitCustomPlot()
{
    jzbind::ClassBind<JZPlotWidget> cls_plot("JZPlotWidget");
    cls_plot.def("clear", true, &JZPlotWidget::clear);
    cls_plot.def("addData",true, &JZPlotWidget::addData);
    cls_plot.regist();

    jzbind::registFunction("initPlotConfig", true, initPlotConfig);

    JZNodeFactory::instance()->registNode(Node_plotConfig, createJZNode<JZNodePlotConfig>);
    //JZNodeEditorManager::instance()->registCustomFunctionNode("initPlotConfig", Node_plotConfig);
}