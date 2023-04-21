#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QWebEngineView>
#include <fstream>
#include <iostream>
#include <string>
#include <Qfile>
#include <QString>
#include <QList>
#include <QtCharts/QChartView>
#include <QtNetwork>
#include <QMainWindow>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QtCharts>
#include <QQueue>
#include <QJsonDocument>
#include <QSizePolicy>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class MessageThread : public QThread
{
    Q_OBJECT
public:
    explicit MessageThread(QObject *parent = nullptr);
    void run() override;
    void addMessage(const QString& message);

signals:
    void messageReceived(const QString& message);

private:
    QMutex mutex;
    QQueue<QString> messageQueue;
};

class Dialog : public QDialog
{
    Q_OBJECT
    QStringList g_num={"광산구"};
    QStringList n_num={"남구"};
    QStringList d_num={"동구"};
    QStringList b_num={"북구"};
    QStringList s_num={"서구"};
public:
    Dialog(QWidget *parent = nullptr);
    QChartView *m_chartView;
    void updateTable(const QByteArray& data);
    ~Dialog();

public slots:
    void write(QString file, QStringList arpr, QStringList maemul);
    void read(QString file);
    void set_chart1(QStringList m_num);
    void set_chart2(void);

    void set_chart3(QStringList d_num);

    void request_data2(void);

private slots:
    QString site(QStringList arpr, QStringList maemul);
    void play_ui(QStringList arpr, QStringList maemul);

    void on_go_btn_clicked();
    void on_chat_btn_clicked();
    void onMessageReceived(const QString& message);
    void on_stick_btn_clicked();
    void on_request_btn_clicked();

    void on_end_btn_clicked();
    void on_pushButton_clicked(); //그래프 보여주기
    void on_combo_region2_activated(int index); //그래프3
    void on_next_Btn_clicked();
    void on_before_Btn_clicked();
private:
    Ui::Dialog *ui;
    QTcpSocket* socket;
    MessageThread messageThread;
};

#endif // DIALOG_H
