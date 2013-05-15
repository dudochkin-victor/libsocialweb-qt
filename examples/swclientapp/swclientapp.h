#ifndef SWCLIENTAPP_H
#define SWCLIENTAPP_H

#include <QObject>
#include <QtDBus>
#include <libsocialweb-qt/swclient.h>
#include <libsocialweb-qt/swclientitem.h>
#include <libsocialweb-qt/swclientitemview.h>
#include <libsocialweb-qt/swclientdbustypes.h>
#include <libsocialweb-qt/swclientserviceconfig.h>

class SwClientApp : public QObject
{
Q_OBJECT
public:
    explicit SwClientApp(QObject *parent = 0);

signals:

public slots:
    void CloseUpShop();

private slots:
    void onItemsAdded(QList<SwClientItem *> itemsAdded);
    void onUserChanged(SwClientService *swService);
    void onDynCapsChanged(SwClientService *swService, QStringList newDynCaps);

private:
    SwClient *swClient;
    QMap<QString, SwClientItemView *> swViews;

};

#endif // SWCLIENTAPP_H
