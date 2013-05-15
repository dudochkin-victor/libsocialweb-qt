#include <QCoreApplication>
#include "swcolltest.h"


int main(int argc, char *argv[]){
    QCoreApplication *app = new QCoreApplication(argc, argv);
    if (app->arguments().count() < 3) {
        qDebug() << QString("Invalid syntax - usage: %1 <servicename> <getlist/getdetails/create> [options]").arg(app->arguments().at(0));
        return -1;
    }
    SwCollTest *swColl = new SwCollTest();

    QObject::connect(app,
            SIGNAL(aboutToQuit()),
            swColl,
            SLOT(CloseUpShop()));

    return app->exec();
}
