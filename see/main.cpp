#include "see.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    See server;
    server.listen(QHostAddress("10.10.21.130"), 9100);

    return app.exec();
}
