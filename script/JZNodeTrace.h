#ifndef JZNODE_TRACE_H_
#define JZNODE_TRACE_H_

#include <QObject>
#include <QSharedPointer>
#include "JZNodeCompiler.h"

class JZScriptEnvironment;
class JZNodeTraceLog
{
public:
    enum Type{
        Push,
        Pop,
        Mark,
        Custom,
    };

    Type type;
    int thread;
    QString name;
    qint64 timestamp;   //ns
};

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
    JZNodeTraceItem();

    int thread;
    int type;
    int level;
    QString name;
    qint64 start;
    qint64 duration;
};

//JZNodeTraceRecord
class JZNodeTraceRecord
{
public:
    void clear();

    QMap<int, QString> threadMap;
    QList<JZNodeTraceItem> items;
};

//JZNodeTraceContext
class JZNodeTraceContext : public QObject
{
    Q_OBJECT

public:
    JZNodeTraceContext();
    ~JZNodeTraceContext();

    const JZNodeTraceRecord& record() const;

    void insert(JZNodeTraceLog item);
    void clear();

    JZNodeTraceRecord parse();
    bool load(QString path);
    bool save(QString path);

signals:
    void sigTrace(const JZNodeTraceLog&item);

protected:
    QList<JZNodeTraceLog> m_items;
    JZNodeTraceRecord m_record;
};
void JZNodeTraceInit(JZScriptEnvironment *env);

#endif
