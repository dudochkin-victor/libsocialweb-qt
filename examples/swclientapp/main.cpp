#include <QCoreApplication>
#include "swclientapp.h"


int main(int argc, char *argv[]){
    QCoreApplication *app = new QCoreApplication(argc, argv);
    SwClientApp *swApp = new SwClientApp();

    QObject::connect(app,
            SIGNAL(aboutToQuit()),
            swApp,
            SLOT(CloseUpShop()));

    //swApp->doStuff();
    return app->exec();
}
