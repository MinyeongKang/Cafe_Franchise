#include "see.h"
#include "ui_widget.h"

#include <QSqlQuery>

See::See(QObject *parent) : QTcpServer(parent)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("10.10.21.123");
    db.setDatabaseName("franchise");
    db.setUserName("fcha");
    db.setPassword("0000");
    if (!db.open())
    {
        qDebug() << "Failed to connect to database.";
        return;
    }
    qDebug() << "Connected to database!";
    connect(this, &QTcpServer::newConnection, this, &See::onNewConnection);
}

void See::onNewConnection()
{
    QTcpSocket* socket = nextPendingConnection();
    clients.append(socket);
    connect(socket, &QTcpSocket::readyRead, this, &See::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &See::onDisconnected);
    qDebug() << "New client connected.";
}

void See::onReadyRead()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    QByteArray data = socket->readAll();
    qDebug() << "Server received data: " << data;
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray array = doc.array();
    QString identy = array.at(0).toString();

    if (identy == "매물")
    {
        QString total = array.at(1).toString();
        QString region = array.at(2).toString();
        QString deposit = array.at(3).toString();
        qDebug() << "식별문자: " << identy << ", 자본금: " << total<<", 지역: "<<region<<", 보증금: "<<deposit;
        qDebug() <<" ";

        QSqlQuery query(db);
        QString queryString;
//        SELECT * FROM rent WHERE region='광산구' and deposit>5000 and deposit <10000 ORDER BY grade DESC LIMIT 10;
        if (deposit == "5000")
        {
            queryString = QString("SELECT * FROM rent WHERE region='%1' and deposit<=%2 ORDER BY grade DESC LIMIT 10").arg(region).arg(deposit);
        }
        else
        {
            int a = deposit.toInt();
            queryString = QString("SELECT * FROM rent WHERE region='%1' and %2-5000<deposit and deposit<=%2 ORDER BY grade DESC LIMIT 10").arg(region).arg(a);
        }
        if (!query.exec(queryString))
        {
            qDebug() << "Failed to execute query:" << query.lastError().text();
            db.close();
            QJsonObject response;
            response.insert("type", "error");
            response.insert("message", "Failed to execute query");
            socket->write(QJsonDocument(response).toJson());


            return;
        }


        QJsonArray resultArray;
        while (query.next())
        {
//            rent 테이블의 실제 보낼 칼럼값 5개

            QString add = query.value("address").toString();
            QString dep = query.value("deposit").toString();
            QString cos = query.value("cost").toString();
            QString grade = query.value("grade").toString();
            QString possible = query.value("possible").toString();

            QJsonObject resultObject;
            resultObject.insert("address", add);
            resultObject.insert("deposit", dep);
            resultObject.insert("cost", cos);
            resultObject.insert("grade", grade);
            resultObject.insert("possible", possible);
            resultArray.append(resultObject);
        }
        QJsonObject response;
        response.insert("type", "result");
        response.insert("data", resultArray);

        qDebug()<<"서버에서 보낸 값은: "<<response;
        socket->write(QJsonDocument(response).toJson());
    }

    else if(identy == "결정")
    {
        QString address = array.at(1).toString();
        QString deposit_two = array.at(2).toString();
        QString cost = array.at(3).toString();
        QString total_money = array.at(4).toString();

        int remain_money = total_money.toInt() - deposit_two.toInt() - (cost.toInt()*10);


        qDebug() << "식별문자: " << identy <<", 주소:"<<address<< ", 보증금: " << deposit_two<<", 월세: "<<cost<<", 총자본: "<<total_money<<"남은 자금: "<<remain_money;
        qDebug() <<" ";

        QSqlQuery query(db);
        QString queryString;

        queryString = QString("UPDATE rent SET possible='불가능' WHERE address='%1'").arg(address);

        // 쿼리문 실행
        if(query.exec(queryString)) {
            qDebug() << "매물 가능 여부가 변경되었습니다.";
        }
        else {
            qDebug() << "매물 가능 여부 변경 실패: " << query.lastError().text();
            QJsonObject response;
            response.insert("type", "error");
            response.insert("message", "Failed to execute query");
            socket->write(QJsonDocument(response).toJson());
            return;
        }

        if (remain_money < 0)
        {
            qDebug() << "남은자금이 부족합니다";
            return;
        }

        else
        {
            queryString = QString("SELECT * FROM store where price<'%1'").arg(remain_money);

        }



        if (!query.exec(queryString))
        {
            qDebug() << "Failed to execute query:" << query.lastError().text();
            db.close();
            QJsonObject response;
            response.insert("type", "error");
            response.insert("message", "Failed to execute query");
            socket->write(QJsonDocument(response).toJson());


            return;
        }



        QJsonArray resultArray;
        while (query.next())
        {
//            store 테이블의 실제 보낼 칼럼값 2개

            QString name = query.value("name").toString();
            QString price = query.value("price").toString();


            QJsonObject resultObject;

            QString str_remain_money = QString::number(remain_money);
            resultObject.insert("remain_money", str_remain_money);

//            resultObject.insert("remain_money",remain_money);
            resultObject.insert("name", name);
            resultObject.insert("price", price);


            resultArray.append(resultObject);
        }
        QJsonObject response;
        response.insert("type", "result2");
        response.insert("data", resultArray);

        qDebug()<<"서버에서 보낸 값은: "<<response;
        socket->write(QJsonDocument(response).toJson());



    }
    else if(identy == "최종")
    {
        QString user = array.at(1).toString();
        QString company = array.at(2).toString();
        QString available_money = array.at(3).toString();



        qDebug() << "사용자 이름: " << user <<", 프랜차이즈사: "<<company<< ", 가용금액: " << available_money;
        qDebug() <<" ";

        QSqlQuery query(db);
        QString queryString;



        queryString = QString("INSERT INTO final values('%1','%2','%3')").arg(user).arg(company).arg(available_money);

        if (!query.exec(queryString))
        {
            qDebug() << "Failed to execute query:" << query.lastError().text();
            db.close();
            QJsonObject response;
            response.insert("type", "error");
            response.insert("message", "Failed to execute query");
            socket->write(QJsonDocument(response).toJson());


            return;
        }


        queryString = QString("SELECT * FROM final where name='%1'").arg(user);

        if (!query.exec(queryString))
        {
            qDebug() << "Failed to execute query:" << query.lastError().text();
            db.close();
            QJsonObject response;
            response.insert("type", "error");
            response.insert("message", "Failed to execute query");
            socket->write(QJsonDocument(response).toJson());


            return;
        }









        QJsonArray resultArray;
        while (query.next())
        {
//            store 테이블의 실제 보낼 칼럼값 2개

            QString user_name = query.value("name").toString();
            QString company_name = query.value("company").toString();
            QString available = query.value("remain").toString();


            QJsonObject resultObject;


            resultObject.insert("name", user_name);
            resultObject.insert("company", company_name);
            resultObject.insert("remain", available);


            resultArray.append(resultObject);
        }
        QJsonObject response;
        response.insert("type", "result3");
        response.insert("data", resultArray);

        qDebug()<<"서버에서 보낸 값은: "<<response;
        socket->write(QJsonDocument(response).toJson());




    }
    else if (identy == "request"){
        QSqlQuery query(db);
        QString queryString;

        QStringList regions ={"광산구", "남구", "동구", "북구", "서구"};
        QStringList money_range = {"0", "2000", "4000", "6000", "8000", "10000", "12000", "14000", "16000", "18000", "20000"};

        QJsonArray resultArray;
        for(int i=0; i<5; i++){
            queryString = QString("SELECT count(*) FROM rent where region = '%1'");
            query.prepare(queryString.arg(regions[i]));
            query.exec();
            query.next();
//            region_num.append(query.value(0).toString());

            QJsonObject resultObject;

            resultObject.insert("maemul_num", query.value(0).toString());
            resultArray.append(resultObject);
        }
        for(int i=0; i<(money_range.size()-1); i++){

            queryString = QString("SELECT count(*) FROM rent WHERE deposit > %1 and deposit<=%2");
            query.prepare(queryString.arg(money_range[i].toInt()).arg(money_range[i+1].toInt()));
            query.exec();
            query.next();

            QJsonObject resultect;

//            resultObject.insert("district", regions[i]);
            resultect.insert("range_num", query.value(0).toString());
            resultArray.append(resultect);
        }
        QJsonArray Array_third;
        for (int i=0; i<5;i++){
            for (int j=0; j<(money_range.size()-1);j++){
                queryString = QString("SELECT count(*) FROM rent WHERE region = '%1' and deposit > %2 and deposit<=%3");
                query.prepare(queryString.arg(regions[i]).arg(money_range[j].toInt()).arg(money_range[j+1].toInt()));
                query.exec();
                query.next();

                QJsonObject result_three;

                result_three.insert("district", regions[i]);
                result_three.insert("district_num", query.value(0).toString());
                Array_third.append(result_three);
            }
        }
        QJsonObject response;
        response.insert("type", "request");
        response.insert("data", resultArray);
        response.insert("info", Array_third);

        qDebug()<<"서버에서 보낸 값은: "<<response;
        socket->write(QJsonDocument(response).toJson());

    }
    else if(identy=="그래프")
    {
        QString region_three = array.at(1).toString();
        QString deposit_three = array.at(2).toString();
        qDebug() << "식별문자: " << identy <<", 지역: "<<region_three<<", 보증금: "<<deposit_three;
        qDebug() <<" ";

        QSqlQuery query(db);
        QString queryString;
        if (deposit_three == "5000")
        {
            queryString = QString("SELECT * FROM rent WHERE region='%1' and deposit<=%2 ORDER BY grade DESC LIMIT 10").arg(region_three).arg(deposit_three);
        }
        else
        {
            int a = deposit_three.toInt();
            queryString = QString("SELECT * FROM rent WHERE region='%1' and %2-5000<deposit and deposit<=%2 ORDER BY grade DESC LIMIT 10").arg(region_three).arg(a);
        }
        if (!query.exec(queryString))
        {
            qDebug() << "Failed to execute query:" << query.lastError().text();
            db.close();
            QJsonObject response;
            response.insert("type", "error");
            response.insert("message", "Failed to execute query");
            socket->write(QJsonDocument(response).toJson());


            return;
        }


        QJsonArray resultArray;
        while (query.next())
        {

            QString dep2 = query.value("deposit").toString();
            QString cos2 = query.value("cost").toString();


            QJsonObject resultObject;

            resultObject.insert("deposit", dep2);
            resultObject.insert("cost", cos2);


            resultArray.append(resultObject);
        }
        QJsonObject response;
        response.insert("type", "result4");
        response.insert("data", resultArray);

        qDebug()<<"서버에서 보낸 값은: "<<response;
        socket->write(QJsonDocument(response).toJson());
    }

    else
    {
        qDebug()<<"보낼 데이터가 없습니다";
        QJsonObject response;
        response.insert("type", "error");
        response.insert("message", "Invalid data");
        socket->write(QJsonDocument(response).toJson());
        return;
    }


}

void See::onDisconnected()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    clients.removeAll(socket);
    socket->deleteLater();
    qDebug() << "Client disconnected.";
}
