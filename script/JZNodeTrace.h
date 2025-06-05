#ifndef JZNODE_TRACE_H_
#define JZNODE_TRACE_H_

#include <QObject>
#include <QSharedPointer>
#include "JZNodeCompiler.h"

class JZNodeTraceItem
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
typedef QSharedPointer<JZNodeTraceItem> JZNodeTraceItemPtr;

class JZNodeTrace
{
public:
    JZNodeTrace();
    ~JZNodeTrace();

    void mark(const QString &mark);
    void push(const QString &text);
    void pop();

protected:
    JZNodeTraceItemPtr createTrace(JZNodeTraceItem::Type type);
};

class JZNodeTraceBuilder
{
public:
    JZNodeTraceBuilder(JZNodeCompiler *c);
    ~JZNodeTraceBuilder();

    void mark(const QString &mark);
    void push(const QString &text);
    void pop();

    JZNodeCompiler *m_compiler
    int m_id;
};

class JZNodeTraceContext : public QObject
{
    Q_OBJECT

public:
    JZNodeTraceContext();
    ~JZNodeTraceContext();

    void record(JZNodeTraceItemPtr item);
    void clear();

    bool load(QString path);
    bool save(QString path);

signals:
    void sigTrace(const JZNodeTraceItemPtr &item);

protected:
    QList<JZNodeTraceItemPtr> m_items;
};


#endif
