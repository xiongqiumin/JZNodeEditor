#ifndef JZNODE_TRACE_H_
#define JZNODE_TRACE_H_

#include <QObject>
#include <QSharedPointer>
#include "JZNodeCompiler.h"

class JZScriptEnvironment;
class JZNodeTraceRecord
{
public:
    enum Type{
        Push,
        Pop,
        Mark,
        Custom,
    };

    Type type;
    QString name;
    qint64 timestamp;   //ns
};
typedef QSharedPointer<JZNodeTraceRecord> JZNodeTraceRecordPtr;

void JZTracePush(const QString &text);
void JZTracePop();
void JZTraceMark(const QString &text);

class JZTraceScoped
{
public:
    JZTraceScoped(QString nodeName);
    ~JZTraceScoped();
};

class JZNodeTraceBuilder
{
public:
    JZNodeTraceBuilder(JZNodeCompiler *c);
    ~JZNodeTraceBuilder();

    void mark(const QString &mark);
    void push(const QString &text);
    void pop();

protected:
    JZNodeCompiler* m_compiler;    
};


class JZNodeTraceItem
{
public:    
    int type;
    int level;
    QString name;
    qint64 start;
    qint64 duration;
};

class JZNodeTraceContext : public QObject
{
    Q_OBJECT

public:
    JZNodeTraceContext();
    ~JZNodeTraceContext();

    void record(JZNodeTraceRecordPtr item);
    void clear();

    QList<JZNodeTraceItem> parse();
    bool load(QString path);
    bool save(QString path);

signals:
    void sigTrace(const JZNodeTraceRecordPtr &item);

protected:
    QList<JZNodeTraceRecordPtr> m_items;
};
void JZNodeTraceInit(JZScriptEnvironment *env);

#endif
