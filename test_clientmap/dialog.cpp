#include "dialog.h"
#include "ui_dialog.h"
#include <QWebEngineView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QtCharts>
#include <QVBoxLayout>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    socket = new QTcpSocket(this);
        connect(socket, &QTcpSocket::readyRead, [&]() {
            QByteArray data = socket->readAll();
            messageThread.addMessage(QString::fromUtf8(data));
        });
        connect(&messageThread, &MessageThread::messageReceived, this, &Dialog::onMessageReceived);
        messageThread.start();
//    play_ui();
}

Dialog::~Dialog()
{
    messageThread.quit();
    messageThread.wait();
    delete ui;
}

QString Dialog::site(QStringList arpr, QStringList maemul){
    QString html;
//    QStringList arpr;
//    QStringList maemul;
    QStringList address_arr;
    for(const QString& add_ar:arpr){
        address_arr << "\"" + add_ar + "\"";
    }
    QString formattedArray = "[" + address_arr.join(", ") + "]";
    QStringList sale_arr;
    for(const QString& sale_money:maemul){
        sale_arr << "\"" + sale_money + "\"";
    }
    QString maemul_money = "[" + sale_arr.join(", ") + "]";
    qDebug()<<formattedArray;
    html=QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">

</head>
<body>
<div id="map" style="width:100%;height:410px;"></div>
<div id="clickLatlng"></div>

<script type="text/javascript" src="//dapi.kakao.com/v2/maps/sdk.js?appkey=42a272db7e092bdf5e18a21b11d8b9c4&libraries=services"></script>
<script>
var mapContainer = document.getElementById("map"), // 지도를 표시할 div
    mapOption = {
        center: new kakao.maps.LatLng(35.13288717306681, 126.90221675003907), // 지도의 중심좌표
        level: 5 // 지도의 확대 레벨
    };

var selectedMarker = null;

var map = new kakao.maps.Map(mapContainer, mapOption); // 지도를 생성합니다

var geocoder = new kakao.maps.services.Geocoder();
var basicSrc ="http://t1.daumcdn.net/localimg/localimages/07/2018/pc/img/marker_spot.png",
    basicSize = new kakao.maps.Size(72, 100);
var basicImage = new kakao.maps.MarkerImage(basicSrc, basicSize);
var imageSrc = 'https://t1.daumcdn.net/localimg/localimages/07/mapapidoc/markerStar.png',
    imageSize = new kakao.maps.Size(24, 35);
var markerImage = new kakao.maps.MarkerImage(imageSrc, imageSize);

var sale_address = %1;
var sale_price = %2;
var num = 0;
sale_address.forEach(function(addr, index){
    geocoder.addressSearch(addr, function(result, status) {
        if (status === kakao.maps.services.Status.OK) {

            var coords = new kakao.maps.LatLng(result[0].y, result[0].x);

            var marker = new kakao.maps.Marker({
                position: coords,
                clickable: true
            });

            marker.setMap(map);

            var iwRemoveable = true;


            // 인포윈도우를 생성합니다
            var infowindow = new kakao.maps.InfoWindow({
                content : '<div style="width: 250px; padding:6px;">'+ sale_address[index]+'<br>'+ sale_price[index]+ '</div>',
//                removable : iwRemoveable
            });
            num+=1;

            kakao.maps.event.addListener(marker, 'mouseover', makeOverListener(map, marker, infowindow));
            kakao.maps.event.addListener(marker, 'mouseout', makeOutListener(infowindow, marker));
            kakao.maps.event.addListener(marker, 'click', makeclicklistener(coords, map, marker));

            // 지도의 중심을 결과값으로 받은 위치로 이동시킵니다
            map.setCenter(coords);
        }
    });
});
function makeclicklistener(coords, map, marker){
    return function() {
        if (!selectedMarker || selectedMarker !== marker) {

            // 클릭된 마커 객체가 null이 아니면
            // 클릭된 마커의 이미지를 기본 이미지로 변경하고
            !!selectedMarker && selectedMarker.setImage(selectedMarker.basicImage);

            // 현재 클릭된 마커의 이미지는 클릭 이미지로 변경합니다
            marker.setImage(markerImage);
        }
        selectedMarker = marker;
    };
}

function makeOverListener(map, marker, infowindow) {
    return function() {
        if (!selectedMarker || selectedMarker !== marker) {
            infowindow.open(map, marker);
        }
    };
}

// 인포윈도우를 닫는 클로저를 만드는 함수입니다
function makeOutListener(infowindow, marker) {
    return function() {
        infowindow.close();

    };
}
</script>
</body>
</html>)").arg(formattedArray, maemul_money);
    return html;
}

void Dialog::write(QString filename, QStringList arpr, QStringList maemul)
{
    QFile file(filename);
    // Trying to open in WriteOnly and Text mode
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug() << " Could not open file for writing";
        return;
    }
    QTextStream out(&file);
    out << site(arpr, maemul);

    file.flush();
    file.close();
}

void Dialog::read(QString filename)
{
    QFile file(filename);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << " Could not open the file for reading";
        return;
    }

    QTextStream in(&file);
    QString myText = in.readAll();
    qDebug() << myText;

    file.close();
}

void Dialog::play_ui(QStringList arpr, QStringList maemul){
    QString filename = "C:/Qt/dsa.html";
    write(filename, arpr, maemul);
    if(ui->verticalLayout->count()>0)
        delete ui->verticalLayout->takeAt(0);
    QWebEngineView *view = new QWebEngineView();
    view->setUrl(QUrl("http://localhost/Qt/dsa.html"));
    ui->verticalLayout->addWidget(view);

}


void Dialog::on_go_btn_clicked()
{
    QString ip = "10.10.21.130";
    int port = 9100;

    socket->connectToHost(ip, port);

    if(socket->waitForConnected())
    {
        qDebug() << "Connected to server!";
    }
    else
    {
        qDebug() << "Failed to connect to server.";
    }
    ui->go_btn->hide();
    request_data2();
}

void Dialog::on_chat_btn_clicked()
{
      ui->label->setText(" ");

//    QString data = ui->liner->text();
//    socket->write(data.toUtf8());
      QString identy = "매물";
      QString region = ui->combo_region->currentText(); // 콤보박스에서 선택한 지역
      QString deposit = ui->combo_money->currentText(); // 콤보박스에서 선택한 보증금
//      총 자본금 = total
      QString total= ui->liner->text();
      qDebug()<<"선택한 지역은: "<<region<<",선택한 보증금은: "<<deposit;
//      QList<QString> data;
      QJsonArray data;
      data.append(identy);
      data.append(total);
      data.append(region);
      data.append(deposit);

//      data를 json으로 변환후, 전송, QJsonDocument::fromVariant함수는 Qvariant 타입에서
//      json객체를 생성, toJson함수는 json객체를 문자열로 변환
//      socket->write(QJsonDocument::fromVariant(data).toJson());
      socket->write(QJsonDocument(data).toJson());
      qDebug()<<"보낸 제이슨 데이터: "<<data;
      ui->liner->clear();
      updateTable(socket->readAll());

}

void Dialog::updateTable(const QByteArray& data)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = jsonResponse.object();

    if (jsonObject.value("type") == "result")
    {
        QJsonArray dataArray = jsonObject.value("data").toArray();
        for (int i = 0; i < dataArray.size(); ++i) {
            QJsonObject dataObject = dataArray[i].toObject();
            QString address = dataObject.value("address").toString();
            QString deposit = dataObject.value("deposit").toString();
            QString cost = dataObject.value("cost").toString();
            QString grade = dataObject.value("grade").toString();
            QString possible = dataObject.value("possible").toString();


            // 테이블에 데이터가 이미 있는지 확인
            int row = -1;
            for (int j = 0; j < ui->table->rowCount(); j++) {
                QTableWidgetItem* itemAddress = ui->table->item(j, 0);
                QTableWidgetItem* itemDeposit = ui->table->item(j, 1);
                QTableWidgetItem* itemGrade = ui->table->item(j, 2);
                if (itemAddress && itemDeposit && itemGrade &&
                    itemAddress->text() == address &&
                    itemDeposit->text() == deposit &&
                    itemGrade->text() == grade) {
                    row = j;
                    break;
                }
            }

            if (row == -1) {
                // 테이블에 데이터가 없으면 추가
                int rowCount = ui->table->rowCount();
                ui->table->insertRow(rowCount);
                QTableWidgetItem* itemAddress = new QTableWidgetItem(address);
                QTableWidgetItem* itemDeposit = new QTableWidgetItem(deposit);
                QTableWidgetItem* itemCost = new QTableWidgetItem(cost);
                QTableWidgetItem* itemGrade = new QTableWidgetItem(grade);
                QTableWidgetItem* itemPossible = new QTableWidgetItem(possible);

                ui->table->setItem(rowCount, 0, itemAddress);
                ui->table->setItem(rowCount, 1, itemDeposit);
                ui->table->setItem(rowCount, 2, itemCost);
                ui->table->setItem(rowCount, 3, itemGrade);
                ui->table->setItem(rowCount, 4, itemPossible);
            } else {
                // 테이블에 데이터가 있으면 업데이트

                QTableWidgetItem* itemAddress = ui->table->item(row, 0);
                QTableWidgetItem* itemDeposit = ui->table->item(row, 1);
                QTableWidgetItem* itemCost = ui->table->item(row, 2);
                QTableWidgetItem* itemGrade = ui->table->item(row, 3);
                QTableWidgetItem* itemPossible = ui->table->item(row, 4);


                itemAddress->setText(address);
                itemDeposit->setText(deposit);
                itemCost->setText(cost);
                itemGrade->setText(grade);
                itemPossible->setText(possible);
            }
        }
    }
}



void Dialog::onMessageReceived(const QString &message)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();

//    qDebug() << "Received message: " << message;
    if (jsonObject.value("type") == "result")
    {
        QJsonArray dataArray = jsonObject.value("data").toArray();

        // 테이블 초기화
        ui->table->clear();
        ui->table->setRowCount(0);
        ui->table->setColumnCount(5);
        ui->table->setHorizontalHeaderLabels({"주소", "보증금", "월세", "점수","계약가능여부"});

        QStringList address_list;
        QStringList money_list;
        for (int i = 0; i < dataArray.size(); ++i)
        {
            QJsonObject dataObject = dataArray[i].toObject();
            QString address = dataObject.value("address").toString();
            QString deposit = dataObject.value("deposit").toString();
            QString cost = dataObject.value("cost").toString();
            QString grade = dataObject.value("grade").toString();
            QString possible = dataObject.value("possible").toString();

            int row = ui->table->rowCount();
            ui->table->insertRow(row);

            QTableWidgetItem* itemAddress = new QTableWidgetItem(address);
            QTableWidgetItem* itemDeposit = new QTableWidgetItem(deposit);
            QTableWidgetItem* itemCost = new QTableWidgetItem(cost);
            QTableWidgetItem* itemGrade = new QTableWidgetItem(grade);
            QTableWidgetItem* itemPossible = new QTableWidgetItem(possible);

            ui->table->setItem(row, 0, itemAddress);
            ui->table->setItem(row, 1, itemDeposit);
            ui->table->setItem(row, 2, itemCost);
            ui->table->setItem(row, 3, itemGrade);
            ui->table->setItem(row, 4, itemPossible);
            address_list.append(address);
            money_list.append(deposit+"/"+cost);
        }
        qDebug()<<address_list;
        play_ui(address_list, money_list);
    }
    else if(jsonObject.value("type") == "result2")
    {
        qDebug()<<"result2!!!";
        QJsonArray dataArray = jsonObject.value("data").toArray();

        ui->table->clear();

        ui->table->setRowCount(0);
        ui->table->setColumnCount(3);
        ui->table->setHorizontalHeaderLabels({"프랜차이즈사이름", "창업비용","최종여유자금"});
        for (int i = 0; i < dataArray.size(); ++i)
        {
            QJsonObject dataObject = dataArray[i].toObject();
            QString remain_money = dataObject.value("remain_money").toString();
            // remain_money가 비어 있는 경우 "0"으로 초기화합니다.
            if (remain_money.isEmpty())
            {
                remain_money = "0";
            }


            QString text_money = QString("남은금액: %1").arg(remain_money);

        // QLabel의 텍스트 색상을 검정색으로 설정합니다.
            ui->label->setStyleSheet("QLabel { color : black; }");

           // QLabel의 크기를 충분히 크게 설정합니다.
            ui->label->setMinimumSize(100, 50);



            ui->label->setText(text_money);


            QString name = dataObject.value("name").toString();
            QString price = dataObject.value("price").toString();


            int row = ui->table->rowCount();
            ui->table->insertRow(row);
            QTableWidgetItem* itemName = new QTableWidgetItem(name);
            QTableWidgetItem* itemPrice = new QTableWidgetItem(price);
            QTableWidgetItem* itemRemain_money = new QTableWidgetItem(remain_money);

            ui->table->setItem(row, 0, itemName);
            ui->table->setItem(row, 1, itemPrice);
            ui->table->setItem(row, 2, itemRemain_money);
        }




    }

    else if(jsonObject.value("type") == "result3")
    {
        qDebug()<<"result3!!!";
        QJsonArray dataArray = jsonObject.value("data").toArray();

        ui->table->clear();
        ui->table->setRowCount(0);
        ui->table->setColumnCount(3);
        ui->table->setHorizontalHeaderLabels({"성함", "프랜차이즈사","여유자금"});
        for (int i = 0; i < dataArray.size(); ++i)
        {
            QJsonObject dataObject = dataArray[i].toObject();

            ui->label->setText(" ");

            QString name = dataObject.value("name").toString();
            QString company = dataObject.value("company").toString();
            QString remain = dataObject.value("remain").toString();

            int row = ui->table->rowCount();
            ui->table->insertRow(row);
            QTableWidgetItem* itemName = new QTableWidgetItem(name);
            QTableWidgetItem* itemCompany = new QTableWidgetItem(company);
            QTableWidgetItem* itemRemain = new QTableWidgetItem(remain);

            ui->table->setItem(row, 0, itemName);
            ui->table->setItem(row, 1, itemCompany);
            ui->table->setItem(row, 2, itemRemain);
        }
    }

    else if(jsonObject.value("type") == "result4")
    {
        qDebug()<<"result4!!!";
        QJsonArray dataArray = jsonObject.value("data").toArray();

        if(ui->verticalLayout_2->count()>0)
            delete ui->verticalLayout_2->takeAt(0);



        // 데이터를 생성합니다.
        QBarSet *set = new QBarSet("Data");
        QStringList categories;

        for (int i = 0; i < dataArray.size(); ++i)
        {
            QJsonObject dataObject = dataArray[i].toObject();


            QString deposit = dataObject.value("deposit").toString();
            QString cost = dataObject.value("cost").toString();
            int ratio = (cost.toInt()*100/deposit.toInt());
            set->append(ratio);

            // x축 라벨을 추가합니다.
            QString category = dataObject.value("category").toString();
            categories.append(category);
        }

        // 데이터를 시리즈에 추가합니다.
        QBarSeries *series = new QBarSeries;
        series->append(set);

        // 시리즈를 차트에 추가합니다.
        QChart *chart = new QChart;
        chart->addSeries(series);

        // x축 라벨을 설정합니다.
        QBarCategoryAxis *axis = new QBarCategoryAxis();
        axis->append(categories);
        chart->addAxis(axis, Qt::AlignBottom);
        series->attachAxis(axis);



        // y축 범위를 설정합니다.
        QValueAxis *valueAxis = new QValueAxis();
        valueAxis->setRange(0, 20);
        valueAxis->setTitleText("(단위:%)"); // y축에 (단위:%) 추가
        chart->addAxis(valueAxis, Qt::AlignLeft);
        series->attachAxis(valueAxis);


        // 차트의 타이틀을 설정합니다.
        chart->setTitle("보증금에 따른 월세 비율");

        // 차트의 레이아웃을 설정합니다.
        chart->legend()->setVisible(false);
        chart->setAnimationOptions(QChart::SeriesAnimations);


        // 차트를 뷰에 설정합니다.
        m_chartView = new QChartView(this);
        m_chartView->setRenderHint(QPainter::Antialiasing);

        // 차트를 위젯에 배치합니다.
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(m_chartView);
        setLayout(layout);
        m_chartView->setChart(chart);

        m_chartView->setVisible(true);
        // 차트 뷰를 Vertical Layout에 추가합니다.

        ui->verticalLayout_2->addWidget(m_chartView);







    }

    else if (jsonObject.value("type") == "request")
    {
        QStringList rgn_num;
        QStringList m_num;
        QJsonArray dataArray = jsonObject.value("data").toArray();
        qDebug()<< dataArray;
        for (int i = 0; i < dataArray.size(); ++i) {
            QJsonObject dataObject = dataArray[i].toObject();
            if (dataObject.value("maemul_num").toString() != ""){
                m_num.append(dataObject.value("maemul_num").toString());
             }
            else if (dataObject.value("range_num").toString() != ""){
                rgn_num.append(dataObject.value("range_num").toString());
            }
        }
        set_chart1(m_num);
//        qDebug()<<rgn_num;
        QJsonArray dataArray1 = jsonObject.value("info").toArray();
        qDebug()<< dataArray1;
        for (int i = 0; i < dataArray1.size(); ++i) {
            QJsonObject dataObject = dataArray1[i].toObject();
            if (dataObject.value("district").toString() == "광산구"){
                g_num.append(dataObject.value("district_num").toString());
            }
            else if (dataObject.value("district").toString() == "남구")
                n_num.append(dataObject.value("district_num").toString());

            else if (dataObject.value("district").toString() == "동구")
                d_num.append(dataObject.value("district_num").toString());

            else if (dataObject.value("district").toString() == "북구")
                b_num.append(dataObject.value("district_num").toString());

            else if (dataObject.value("district").toString() == "서구")
                s_num.append(dataObject.value("district_num").toString());
        }
        QBarSet *set0 = new QBarSet("매물 수");

        *set0 << rgn_num[0].toInt() << rgn_num[1].toInt() << rgn_num[2].toInt() << rgn_num[3].toInt() << rgn_num[4].toInt() << rgn_num[5].toInt() << rgn_num[6].toInt() << rgn_num[7].toInt() << rgn_num[8].toInt() << rgn_num[9].toInt();

        QBarSeries *series2 = new QBarSeries();
        series2->append(set0);

        QChart *chart = new QChart();
        chart->addSeries(series2);
        chart->setTitle("광주 보증금 범위에 따른 매물 수");
        chart->setAnimationOptions(QChart::SeriesAnimations);

        QStringList categories;
        categories << "2천만" << "4천만" << "6천만" << "8천만" << "1억" << "1억2천만" << "1억4천만" << "1억6천만" << "1억8천만" << "2억";
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        chart->addAxis(axisX, Qt::AlignBottom);
        series2->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0,550);
        chart->addAxis(axisY, Qt::AlignLeft);
        series2->attachAxis(axisY);

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        ui->chartLayout2->addWidget(chartView);
        qDebug()<<d_num;

    }
}

MessageThread::MessageThread(QObject *parent) : QThread(parent)
{

}

void MessageThread::run()
{
    while(true)
    {
        mutex.lock();
        if(!messageQueue.isEmpty())
        {
            QString message = messageQueue.dequeue();
            mutex.unlock();
            emit messageReceived(message);
        }
        else
        {
            mutex.unlock();
            msleep(100);
        }
    }
}




void MessageThread::addMessage(const QString &message)
{
    QMutexLocker locker(&mutex);
    messageQueue.enqueue(message);
}


void Dialog::on_request_btn_clicked()
{
    QString identy = "결정";
    QString total= ui->liner->text();

    int row = ui->table->currentRow(); // 선택된 행의 인덱스 가져오기
    if (row < 0)
    {
        QMessageBox::warning(this, "오류", "선택된 행이 없습니다.");
        return;
    }




    QString address, depositValue, monthlyRent, grade, possible;

    address = ui->table->item(row, 0)->text();
    depositValue = ui->table->item(row, 1)->text();
    monthlyRent = ui->table->item(row, 2)->text();
    grade = ui->table->item(row, 3)->text();
    possible = ui->table->item(row, 4)->text();

    qDebug() << "보낸 주소: " << address;
    if (possible == "불가능") {
        QMessageBox::information(this, "알림", "계약 불가능합니다.");
        return;
    }


    // 데이터를 JSON 형식으로 변환해서 소켓을 통해 전송
    QJsonArray data;

    data.append(identy);
    data.append(address);
    data.append(depositValue);
    data.append(monthlyRent);
    data.append(total);
    socket->write(QJsonDocument(data).toJson());
    qDebug() << "보낸 JSON 데이터: " << data;

    ui->liner->clear();


}

void Dialog::on_stick_btn_clicked()
{
    //식별문자
    QString identy = "그래프";
    QString region = ui->combo_region->currentText(); // 콤보박스에서 선택한 지역
    QString deposit = ui->combo_money->currentText(); // 콤보박스에서 선택한 보증금


    qDebug()<<"선택한 지역은: "<<region<<",선택한 보증금은: "<<deposit;
  // 제이슨 어레이는 JSON데이터의 배열 나타날 때 사용
  // QJsonDocument로 JSON데이터 파싱, 배열의 각 요소는 QJsonValue객체로 나타남
  // 인덱스를 사용하여 배열의 각 요소에 액세스 가능

    QJsonArray data;
    data.append(identy);
    data.append(region);
    data.append(deposit);


    socket->write(QJsonDocument(data).toJson());
    qDebug()<<"보낸 제이슨 데이터: "<<data;


}
void Dialog::on_end_btn_clicked()
{
    QString identy = "최종";
    QString user= ui->liner->text();
    int row = ui->table->currentRow(); // 선택된 행의 인덱스 가져오기
    if (row < 0)
    {
        QMessageBox::warning(this, "오류", "선택된 행이 없습니다.");
        return;
    }

    QString name, price, remain_money;

    name = ui->table->item(row,0)->text();
    price = ui->table->item(row,1)->text();
    remain_money = ui->table->item(row,2)->text();
    int available_money = remain_money.toInt() - price.toInt();
    QString str_available_money = QString::number(available_money);

    QJsonArray data;
    data.append(identy);
    data.append(user);
    data.append(name);
    data.append(str_available_money);


    socket->write(QJsonDocument(data).toJson());
    qDebug() << "보낸 JSON 데이터: " << data;

    ui->liner->clear();
}

// 0페이지 그래프
void Dialog:: request_data2()
{
    QJsonArray data;
    data.append("request");
    socket->write(QJsonDocument::fromVariant(data).toJson());
    socket->write(QJsonDocument(data).toJson());
}
void Dialog::on_pushButton_clicked()
{
    set_chart2();
}
void Dialog:: set_chart1(QStringList m_num)
{
    QString region_list = "광산구;남구;동구;북구;서구";
    QStringList regions = region_list.split(";");

    QPieSeries *series1 = new QPieSeries();
    for(int i=0; i<5; i++){
        series1->append(regions[i], m_num[i].toInt());
    }

    QPieSlice *G_lgd = series1->slices().at(0);
    G_lgd->setLabel(QString("광산구, %1%").arg(100*G_lgd->percentage(), 0, 'f', 1));
    G_lgd->setLabelFont(QFont("Arial", 8));
//    G_lgd->setLabelPosition(QPieSlice::LabelInsideHorizontal);
    G_lgd->setLabelVisible();

    QPieSlice *N_lgd = series1->slices().at(1);
    N_lgd->setLabel(QString("남구, %1%").arg(100*N_lgd->percentage(), 0, 'f', 1));
    N_lgd->setLabelFont(QFont("Arial", 8));
    N_lgd->setLabelVisible();

    QPieSlice *D_lgd = series1->slices().at(2);
    D_lgd->setLabel(QString("동구, %1%").arg(100*D_lgd->percentage(), 0, 'f', 1));
    D_lgd->setLabelFont(QFont("Arial", 8));
    D_lgd->setLabelVisible();

    QPieSlice *B_lgd = series1->slices().at(3);
    B_lgd->setLabel(QString("북구, %1%").arg(100*B_lgd->percentage(), 0, 'f', 1));
    B_lgd->setLabelFont(QFont("Arial", 8));
    B_lgd->setLabelVisible();

    QPieSlice *S_lgd = series1->slices().at(4);
    S_lgd->setLabel(QString("서구, %1%").arg(100*S_lgd->percentage(), 0, 'f', 1));
    S_lgd->setLabelFont(QFont("Arial", 8));
    S_lgd->setLabelVisible();

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series1);
    chart->createDefaultAxes();
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->chartLayout1->addWidget(chartView);
}

void Dialog:: set_chart2()
{
//    QByteArray recv_data = socket->readAll();
//    QJsonDocument jsonResponse = QJsonDocument::fromJson(recv_data);
//    QJsonObject jsonObject = jsonResponse.object();

//    QStringList rgn_num;
//    QStringList m_num;

//    if (jsonObject.value("type") == "request")
//    {
//        QJsonArray dataArray = jsonObject.value("data").toArray();
//        qDebug()<< dataArray;
//        for (int i = 0; i < dataArray.size(); ++i) {
//            QJsonObject dataObject = dataArray[i].toObject();
//            if (dataObject.value("maemul_num").toString() != ""){
//                m_num.append(dataObject.value("maemul_num").toString());
//             }
//            else if (dataObject.value("range_num").toString() != ""){
//                rgn_num.append(dataObject.value("range_num").toString());
//            }
//        }
////        qDebug()<<rgn_num;
//        QJsonArray dataArray1 = jsonObject.value("info").toArray();
//        qDebug()<< dataArray1;
//        for (int i = 0; i < dataArray1.size(); ++i) {
//            QJsonObject dataObject = dataArray1[i].toObject();
//            if (dataObject.value("district").toString() == "광산구"){
//                g_num.append(dataObject.value("district_num").toString());
//            }
//            else if (dataObject.value("district").toString() == "남구")
//                n_num.append(dataObject.value("district_num").toString());

//            else if (dataObject.value("district").toString() == "동구")
//                d_num.append(dataObject.value("district_num").toString());

//            else if (dataObject.value("district").toString() == "북구")
//                b_num.append(dataObject.value("district_num").toString());

//            else if (dataObject.value("district").toString() == "서구")
//                s_num.append(dataObject.value("district_num").toString());
//        }
//        qDebug()<<d_num;
//    }
//    set_chart1(m_num);

//    // 그래프
//    QBarSet *set0 = new QBarSet("매물 수");

//    *set0 << rgn_num[0].toInt() << rgn_num[1].toInt() << rgn_num[2].toInt() << rgn_num[3].toInt() << rgn_num[4].toInt() << rgn_num[5].toInt() << rgn_num[6].toInt() << rgn_num[7].toInt() << rgn_num[8].toInt() << rgn_num[9].toInt();

//    QBarSeries *series2 = new QBarSeries();
//    series2->append(set0);

//    QChart *chart = new QChart();
//    chart->addSeries(series2);
//    chart->setTitle("광주 보증금 범위에 따른 매물 수");
//    chart->setAnimationOptions(QChart::SeriesAnimations);

//    QStringList categories;
//    categories << "2천만" << "4천만" << "6천만" << "8천만" << "1억" << "1억2천만" << "1억4천만" << "1억6천만" << "1억8천만" << "2억";
//    QBarCategoryAxis *axisX = new QBarCategoryAxis();
//    axisX->append(categories);
//    chart->addAxis(axisX, Qt::AlignBottom);
//    series2->attachAxis(axisX);

//    QValueAxis *axisY = new QValueAxis();
//    axisY->setRange(0,550);
//    chart->addAxis(axisY, Qt::AlignLeft);
//    series2->attachAxis(axisY);

//    QChartView *chartView = new QChartView(chart);
//    chartView->setRenderHint(QPainter::Antialiasing);
//    ui->chartLayout2->addWidget(chartView);
}

// 그래프3
void Dialog::on_combo_region2_activated(int index)
{
    if(index == 0)
    {
        set_chart3(g_num);
    }
    else if (index ==1)
    {
        set_chart3(n_num);
    }
    else if (index ==2)
    {
        set_chart3(d_num);
    }
    else if (index ==3)
    {
        set_chart3(b_num);
    }
    else
    {
        set_chart3(s_num);
    }
}

void Dialog:: set_chart3(QStringList d_num)
{
    // 그래프
    if(ui->chartLayout3->count()>0)
        delete ui->chartLayout3->takeAt(0);
    QBarSet *set0 = new QBarSet("매물 수");

    *set0 << d_num[1].toInt() << d_num[2].toInt() << d_num[3].toInt() << d_num[4].toInt() << d_num[5].toInt() << d_num[6].toInt() << d_num[7].toInt() << d_num[8].toInt() << d_num[9].toInt() << d_num[10].toInt();

    QBarSeries *series3 = new QBarSeries();
    series3->append(set0);

    QChart *chart = new QChart();
    chart->addSeries(series3);
    chart->setTitle("구별 보증금 범위에 따른 매물 수");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    categories << "2천만" << "4천만" << "6천만" << "8천만" << "1억" << "1억2천만" << "1억4천만" << "1억6천만" << "1억8천만" << "2억";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series3->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0,160);
    chart->addAxis(axisY, Qt::AlignLeft);
    series3->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->chartLayout3->addWidget(chartView);
}

// 버튼 연결 함수
void Dialog::on_next_Btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void Dialog::on_before_Btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}
