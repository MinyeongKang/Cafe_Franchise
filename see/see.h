#ifndef SEE_H
#define SEE_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>


class See : public QTcpServer
{
    Q_OBJECT
public:
    explicit See(QObject *parent = nullptr);

signals:

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QList<QTcpSocket*> clients;
    QSqlDatabase db;
};

#endif // SEE_H
