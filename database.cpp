#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "database.h"

DataBaseConfig::DataBaseConfig()
{
    // 初始化数据库连接
    m_db = QSqlDatabase::addDatabase("QSQLITE");    
}

DataBaseConfig::~DataBaseConfig()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

void DataBaseConfig::init(QString path)
{
    m_db.setDatabaseName(path);
    if (!m_db.open())
    {
        qDebug() << "Open database failed.";
        return;
    }

    // 创建表，只保留 key 字段
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS Configs ("
        "key TEXT PRIMARY KEY, "
        "buffer BLOB)")) {
        qDebug() << "Failed to create table:" << query.lastError().text();
    }
}

bool DataBaseConfig::hasConfig(const QString& key)
{    
    QSqlQuery query;
    query.prepare("SELECT 1 FROM Configs WHERE key = :key");
    query.bindValue(":key", key);
    if (query.exec() && query.next()) {
        return true;
    }
    return false;
}

void DataBaseConfig::addConfig(QString name)
{
    QSqlQuery query;
    // 插入一条初始记录，key 和 buffer 为 NULL
    query.prepare("INSERT OR IGNORE INTO Configs (key, buffer) VALUES (:name, NULL)");
    query.bindValue(":name", name);
    if (!query.exec()) {
        qDebug() << "Failed to add config:" << query.lastError().text();
    }
}

void DataBaseConfig::setConfigBuffer(QString key, const QByteArray &buffer)
{
    QSqlQuery query;
    // 插入或替换记录
    query.prepare("INSERT OR REPLACE INTO Configs (key, buffer) VALUES (:key, :buffer)");
    query.bindValue(":key", key);
    query.bindValue(":buffer", buffer);
    if (!query.exec()) {
        qDebug() << "Failed to set config buffer:" << query.lastError().text();
    }
}

QByteArray DataBaseConfig::getConfigBuffer(QString key)
{
    QByteArray buffer;

    QSqlQuery query;
    // 根据 name 和 key 查询 buffer
    query.prepare("SELECT buffer FROM Configs WHERE key = :key");
    query.bindValue(":key", key);
    if (query.exec() && query.next()) {
        buffer = query.value(0).toByteArray();
    }
    else {
        qDebug() << "Failed to get config buffer:" << query.lastError().text();
    }
    return buffer;
}