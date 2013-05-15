#ifndef SWCOLLTEST_H
#define SWCOLLTEST_H

#include <QObject>
#include <QtDBus>
#include <libsocialweb-qt/swclient.h>
#include <libsocialweb-qt/swclientitem.h>
#include <libsocialweb-qt/swclientitemview.h>
#include <libsocialweb-qt/swclientdbustypes.h>
#include <libsocialweb-qt/swclientserviceconfig.h>

class SwCollTest : public QObject
{
Q_OBJECT
public:
    explicit SwCollTest(QObject *parent = 0);

signals:

public slots:
    void CloseUpShop();

private slots:
    void onCollListRetrieved(SwClientService *svc, ArrayOfSwCollectionDetails list, bool error);
    void onCollDetailsRetrieved(SwClientService *svc, SwCollectionDetails dets, bool error);
    void onCollCreated(SwClientService *svc, QString collID, bool error);

private:
    SwClient *swClient;
    QStringList mArgs;

};

#endif // SWCOLLTEST_H
